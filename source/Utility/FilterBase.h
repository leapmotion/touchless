//  Created by Gabriel Hare on 17/5/12.
//  Copyright 2012 OcuSpec. All rights reserved.

//ABOUT:
//This is the standard interface for all filters.
//It defines standard methods to interact with single object filters.

#ifndef FilterBase_h
#define FilterBase_h

#include <Eigen/Core> //for aligned matrices
#include <climits> //frame_t Step_Frames = (Curr_Frame-Prev_Frame)%frame_m

#define frame_m UINT_MAX
typedef int frame_t;

//NOTE: Aligned (fixed dimension) ~ int > 0, Dynamic ~ int < 0
template <int Dim>
class FilterBase {
public:
  virtual ~FilterBase() {}; //Ensure child destructor called from FilterBase
  //ChildCopy: Enable copying of child classes.
  //Use: Algorithms working with arbitrary filter types
  virtual FilterBase<Dim>* ChildCopy() const = 0;

  //Ready: Returns true when the filter has sufficient data to make a prediction
  virtual bool Ready() const = 0;
  //Reset: Removes all collected data by the filter
  virtual void Reset() = 0;

  //Predict: Predicts the most probable state Pred_Frames into the future,
  //relative to the Frame of the most recent Update call.
  virtual const Eigen::Matrix<double,Dim,1>& Predict(frame_t Pred_Frames) const = 0;
  //Unc2Cov: Gives the covariance tensor for the state predicted Pred_Frames into the future,
  //relative to the Frame of the most recent Update call.
  virtual const Eigen::Matrix<double,Dim,Dim>& Unc2Cov(frame_t Pred_Frames) const = 0;

  //Update: Gives data Meas_State from a state Step_Frames after the previous Update call.
  virtual void Update(frame_t Step_Frames,
                      const Eigen::Matrix<double,Dim,1>& Meas_State,
                      const Eigen::Matrix<double,Dim,Dim>& Meas_C2Unc,
                      double Weight) = 0;

  //Transform: Modify the current state without a Reset & Update
  virtual void Transform(const Eigen::Matrix<double,Dim,Dim>& Mul,
                         const Eigen::Matrix<double,Dim,1>& Add) = 0;
};

#endif
