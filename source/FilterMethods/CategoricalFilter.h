/*==================================================================================================================

    Created on 2013.05.21 by Victor Dods
    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

#ifndef CategoricalFilter_h
#define CategoricalFilter_h

#include <Eigen/Core>
#include <map>
#include <vector>
#include EXCEPTION_PTR_HEADER

#include "FilterBase.h"

template <typename Category_, unsigned int CATEGORY_COUNT>
class CategoricalFilter {
public:

  typedef Category_ Category;
  typedef std::vector<Category> CategoryVector;
  typedef Eigen::Matrix<double, CATEGORY_COUNT, 1> ProbabilityVector;

  // returns a normalized probability vector having equal probabilities for each category (i.e. totally ambgiuous)
  static ProbabilityVector ambiguousProbabilityVector () {
    return ProbabilityVector::Constant(1.0 / CATEGORY_COUNT);
  }

  // the elements of categories must be unique
  CategoricalFilter (CategoryVector const &categories,
                     FilterBase<CATEGORY_COUNT> *filter,
                     double uniformCategorizationThreshold,
                     bool deleteFilterUponDestruction = true)
    :
    m_categories(categories),
    m_filter(filter),
    m_deleteFilterUponDestruction(deleteFilterUponDestruction),
    m_uniformCategorizationThreshold(uniformCategorizationThreshold),
    m_valuesAreCached(false)
  {
    assert(CATEGORY_COUNT > 0);
    assert(m_categories.size() == CATEGORY_COUNT);
    assert(m_filter != NULL);
    assert(0.0 < m_uniformCategorizationThreshold && m_uniformCategorizationThreshold <= 1.0);

    // generate the index map, checking that the elements of the category vector are unique
    unsigned int i = 0;
    for (typename CategoryVector::const_iterator it = m_categories.begin(), it_end = m_categories.end(); it != it_end; ++it) {
      Category const &c = *it;
      if (m_indexMap.find(c) != m_indexMap.end()) {
        throw_rethrowable std::logic_error("the values in the category vector must be unique");
      }
      m_indexMap[c] = i;
      ++i;
    }

    // put the filter into a neutral state -- completely ambiguous probabilities
    reset();
  }
  ~CategoricalFilter ()
  {
    if (m_deleteFilterUponDestruction) {
      delete m_filter;
    }
  }

  // general property accessors

  unsigned int categoryCount () const { return CATEGORY_COUNT; }
  const CategoryVector &categories () const { return m_categories; }
  // will throw if the category is not in the original category vector
  unsigned int indexOfCategory (const Category &c) const { return m_indexMap.at(c); }
  FilterBase<CATEGORY_COUNT> const &getFilter () const { return *m_filter; }
  FilterBase<CATEGORY_COUNT> &getFilter () { return *m_filter; }
  template <typename FilterType>
  FilterType const &getFilterAs () const { return *dynamic_cast<FilterType const *>(m_filter); }
  template <typename FilterType>
  FilterType &getFilterAs () { return *dynamic_cast<FilterType *>(m_filter); }

  // statistical property accessors

  // returns true if there's an unambiguous quantity to return
  bool filteredCategoryIsUnambiguous () const {
    ensureValuesAreCached();
    assert(m_valuesAreCached);
    return m_filteredCategoryIsUnambiguous;
  }
  // throws if filteredCategoryIsUnambiguous returns false, but otherwise returns a valid index
  const Category &filteredCategory () const {
    ensureValuesAreCachedAndFilteredCategoryIsUnambiguous();
    assert(m_filteredCategory != NULL);
    return *m_filteredCategory;
  }
  // returns the vector of probabilities for the vector of categories -- the elements sum to 1.
  const ProbabilityVector &probabilityVector () const {
    ensureValuesAreCached();
    return m_probabilityVector;
  }

  // low-level accessors -- could return max probability and max probability
  // index here, but would rather discourage their use.

  // modifiers/procedures

  // add a measurement to the filter, with an optional weight (default is 1)
  // TODO: could add some scalar uncertainty/confidence to this as well
  void updateWithCategory (const Category &category, double weight = 1.0) {
    // a discrete update is just a continuous update with a completely biased probability vector.
    updateWithProbabilityVector(ProbabilityVector::Unit(indexOfCategory(category)), weight);
  }
  // add a measurement to the filter in the form of a normalized probability vector.
  void updateWithProbabilityVector (ProbabilityVector const &probVec, double weight = 1.0) {
    // TODO: check that probabilityVector is normalized?
    m_filter->Update(1, probVec, UncertaintyMatrix::Zero(), weight);
    m_valuesAreCached = false;
  }
  // resets the filter and sets all probabilities to equal ("completely ambiguous")
  void reset () {
    m_filter->Reset();
    // equal probability for all
    m_filter->Update(1, ambiguousProbabilityVector(), UncertaintyMatrix::Zero(), 1.0);
    assert(m_filter->Ready());
    m_valuesAreCached = false;
  }

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:

  void ensureValuesAreCached () const {
    if (m_valuesAreCached) {
      return;
    }

    assert(m_filter->Ready());
    m_valuesAreCached = true;

    const ProbabilityVector &probabilities = m_filter->Predict(0);
    double normalizationFactor = 0.0;
    for (unsigned int i = 0; i < CATEGORY_COUNT; ++i) {
      assert(0.0 <= probabilities(i));
      normalizationFactor += probabilities(i);
    }
    assert(normalizationFactor > 1e10);

    // the probabilities must sum to 1.
    m_probabilityVector = probabilities / normalizationFactor;

    // determine the maximum probability
    m_maxProbability = 0.0;
    m_maxProbabilityCategory = NULL;
    for (unsigned int i = 0; i < CATEGORY_COUNT; ++i) {
      if (m_maxProbabilityCategory == NULL || m_maxProbability < m_probabilityVector(i)) {
        m_maxProbability = m_probabilityVector(i);
        m_maxProbabilityCategory = &m_categories[i];
      }
    }

    // if the max probability is at the threshold or higher, there is an unambiguous answer
    if (m_maxProbability >= m_uniformCategorizationThreshold) {
      m_filteredCategoryIsUnambiguous = true;
      m_filteredCategory = m_maxProbabilityCategory;
    } else {
      m_filteredCategoryIsUnambiguous = false;
      m_filteredCategory = NULL; // irrelevant value
    }
  }

  void ensureValuesAreCachedAndFilteredCategoryIsUnambiguous () const {
    ensureValuesAreCached();
    if (!m_filteredCategoryIsUnambiguous) {
      throw_rethrowable std::logic_error("don't attempt to call filteredCategoryIndex if filteredCategoryIsUnambiguous returns false");
    }
  }

  typedef std::map<Category,unsigned int> IndexMap;
  typedef Eigen::Matrix<double, CATEGORY_COUNT, CATEGORY_COUNT> UncertaintyMatrix;

  CategoryVector              m_categories; // vector of unique categories
  IndexMap                    m_indexMap;   // given a category, produces its category vector index

  // TODO: filter on N-1 dimensions, using 1-sum(p_i) as the remaining value.  then
  // the normalizing constraint is always satisfied, and there is no wasted dimension.
  FilterBase<CATEGORY_COUNT> *m_filter;
  bool                        m_deleteFilterUponDestruction;
  double                      m_uniformCategorizationThreshold;

  mutable bool                m_valuesAreCached;
  mutable const Category     *m_maxProbabilityCategory;
  mutable double              m_maxProbability; // could return this, but would rather discourage its use
  mutable bool                m_filteredCategoryIsUnambiguous;
  mutable const Category     *m_filteredCategory;
  mutable ProbabilityVector   m_probabilityVector;
};

#endif // CategoricalFilter_h
