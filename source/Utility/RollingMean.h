//  Created by Gabriel Hare on 7/3/13.
//  Copyright (c) 2013 Gabriel Hare. All rights reserved.

//ABOUT: A simple class to measure the mean and uncertainty
//of a stream of positions. This method uses a geometric series
//to define continually updated effective samples weights.
//WARNING: These formulas all assume that the Sampleing series has
//converged. Sufficient convergence is defined by Converge, and can
//be checked using the Ready() method.

#ifndef RollingMean_h
#define RollingMean_h

#include "FilterBase.h"

template <unsigned int dim>
class RollingMean : public FilterBase<dim> {
public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW //Ensure alignment when allocated from heap
  //http://eigen.tuxfamily.org/dox/TopicStructHavingEigenMembers.html
  RollingMean() {
    Sample = 1.; //DEFAULT: No filtering
    Reset();
  };

  virtual FilterBase<dim>* ChildCopy() const {
    return(new RollingMean<dim>(*this));
  };

  //Ready: Returns true when the filter has sufficient data to make a prediction
  virtual bool Ready() const {
    if (1.-Integral > Sample) {
      return(false);
    }
    return(true);
  };
  //Reset: Removes all collected data by the filter
  virtual void Reset() {
    NonZero = false;
    Integral = 0.;
    P1.setZero();
    P2.setZero();
  };

  virtual const Eigen::Matrix<double,dim,1>& Predict(frame_t) const {
    if (!NonZero) {
      C1.setZero();
      return(C1);
    }
    C1 = P1/Integral;
    return(C1);
  };
  virtual const Eigen::Matrix<double,dim,dim>& Unc2Cov(frame_t) const {
    if (!NonZero) {
      C2.setZero();
      return(C2);
    }
    C1 = P1/Integral;
    C2 = P2/Integral - C1*C1.transpose();
    return(C2);
  };

  //Update: Gives data Meas_State from a state Step_Frames after the previous Update call.
  virtual void Update(frame_t,
                      const Eigen::Matrix<double,dim,1>& Meas_State,
                      const Eigen::Matrix<double,dim,dim>& Meas_C2Unc,
                      double Weight) {
    NonZero = true;
    double Filter = Weight*Sample;
    Integral = Filter + (1.-Filter)*Integral;
    P1 = Filter*(Meas_State) + (1.-Filter)*P1;
    P2 = Filter*(Meas_C2Unc + Meas_State*Meas_State.transpose()) + (1.-Filter)*P2;
  };

  virtual void Transform(const Eigen::Matrix<double,dim,dim>& Mul, const Eigen::Matrix<double,dim,1>& Add) {
    P2 = (Mul*P2*Mul.transpose() + (Mul*P1)*Add.transpose() + Add*(Mul*P1).transpose() + Add*Add.transpose()).eval();
    P1 = (Mul*P1 + Add).eval();
    //NOTE: C2' = P2' - P1'*P1'.t() == Mul*P2*Mul.t() - Mul*P1*P1.t()*Mul.t() == Mul*C2*Mul.t()
  };

  void SetWindow(double newWindow) {
    if (1. <= newWindow) {
      Sample = 1./newWindow;
    }
  };

protected:
  double Sample;
  bool NonZero;
  double Integral;
  Eigen::Matrix<double,dim,1> P1;
  Eigen::Matrix<double,dim,dim> P2;

  mutable Eigen::Matrix<double,dim,1> C1;
  mutable Eigen::Matrix<double,dim,dim> C2;
};

#endif
