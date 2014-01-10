// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#include "stdafx.h"
#include "common.h"
#include "Config.h"
#include "API/LeapPluginPlus.h"
#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <stdexcept>

Config::ConfigState& Config::state() {
  static ConfigState s_ConfigState;
  return s_ConfigState;
}

void Config::InitializeDefaults()
{
  static ConfigState& s = state();

  {
    boost::unique_lock<boost::recursive_mutex> lock(s.MapMutex);
    s.AttributeMap.clear();
    //s.SectionToFileMap.clear();
    s.ModifiedMap.clear();
  }

  CreateAttribute("camera_type",                 "UVCI",  WRITE_ALWAYS);
  CreateAttribute("camera_dump_file",                "",  WRITE_NOPUBLIC);
  CreateAttribute("camera_framerate_limit",     1000.0f,  WRITE_ALWAYS);
  CreateAttribute("camera_sensitivity",             1.0,  WRITE_ALWAYS);
  CreateAttribute("camera_requires_calib",         true,  WRITE_ALWAYS);

  CreateAttribute("optical_infinity_value",       161.8,  WRITE_NOPUBLIC);

  CreateAttribute("dump_replay_framerate",           10,  WRITE_NOPUBLIC);
  CreateAttribute("dump_replay_repeat",            true,  WRITE_NOPUBLIC);
  CreateAttribute("dump_replay_min_clients",          0,  WRITE_NOPUBLIC);
  CreateAttribute("dump_replay_max_wait_ms",       1000,  WRITE_NOPUBLIC);
  CreateAttribute("dump_replay_processed_only",    true,  WRITE_NOPUBLIC);

  CreateAttribute("pipeline_stop_requested",      false,  WRITE_NOPUBLIC);
  CreateAttribute("plugin_host_stop_requested",   false,  WRITE_NOPUBLIC);

  CreateAttribute("spherical_wavefront_frequency",  450,  WRITE_NOPUBLIC);
  CreateAttribute("use_wavefront_photon_slowdown", true,  WRITE_NOPUBLIC);

  CreateAttribute("debug_use_image_viewer",       false,  WRITE_NOPUBLIC);
  CreateAttribute("debug_rect_images",             true,  WRITE_NOPUBLIC);

  CreateAttribute("camera_mode",                 "HVGA",  WRITE_ALWAYS);
  CreateAttribute("image_width",                    640,  WRITE_ALWAYS);
  CreateAttribute("image_height",                   480,  WRITE_ALWAYS);
  CreateAttribute("image_source_width",             640,  WRITE_ALWAYS);
  CreateAttribute("image_source_height",            480,  WRITE_ALWAYS);
  CreateAttribute("image_calib_width",              640,  WRITE_ALWAYS);
  CreateAttribute("image_calib_height",             480,  WRITE_ALWAYS);
  CreateAttribute("tracking_mode",            "balanced", WRITE_ALWAYS);

  CreateAttribute("xray_pulse_width",               100, WRITE_NOPUBLIC);

  CreateAttribute("sampling_max_gap",        1.0f,  WRITE_NOPUBLIC);
  CreateAttribute("sampling_min_gap",        0.1f,  WRITE_NOPUBLIC);
  CreateAttribute("sampling_max_gap_robust", 0.8f,  WRITE_NOPUBLIC);
  CreateAttribute("sampling_min_gap_robust", 0.08f, WRITE_NOPUBLIC);

  CreateAttribute("debug_trifocal_tensors",         true, WRITE_NOPUBLIC);

  CreateAttribute("robust_mode_enabled",            true, WRITE_ALWAYS);
  CreateAttribute("low_resource_mode_enabled",     false, WRITE_ALWAYS);

  // 0 = disallow, 1 = ask, 2 = always. should be an enum that lives somewhere sane.
  CreateAttribute("background_app_mode",          1,      WRITE_ALWAYS);

  CreateAttribute("plasmeto_mode_enabled",        false,  WRITE_NOPUBLIC);

  //CreateAttribute("dshow_use_null_filter",        false,  WRITE_NOPUBLIC);
  //CreateAttribute("dshow_use_single_graph_builder", true, WRITE_NOPUBLIC);
  //CreateAttribute("dshow_use_media_time",         false,  WRITE_NOPUBLIC);
  //CreateAttribute("dshow_use_gray_mode_mjpeg",    false,  WRITE_NOPUBLIC);
  //CreateAttribute("dshow_use_sync_graph",          true,  WRITE_NOPUBLIC);

  CreateAttribute("debug_output_level",               3,  WRITE_NOPUBLIC);
  CreateAttribute("debug_output_mode",        "console",  WRITE_NOPUBLIC);
  CreateAttribute("profiling_output_level",           1,  WRITE_NOPUBLIC);
  CreateAttribute("profiling_output_mode",       "none",  WRITE_NOPUBLIC);

  CreateAttribute("websockets_enabled",            true,  WRITE_ALWAYS);
  CreateAttribute("websockets_port",               6437,  WRITE_NOPUBLIC);

  CreateAttribute("server_connect_ip",        LOCALHOST,  WRITE_ALWAYS);

  CreateAttribute("print_timestamps_in_log",       true,  WRITE_ALWAYS);

  CreateAttribute("use_interference_fringing_pattern", true, WRITE_NOPUBLIC);

  // filtering params
  CreateAttribute("filtering_enabled",              true, WRITE_NOPUBLIC);

  CreateAttribute("filtering_scale_posunc",           1., WRITE_NOPUBLIC);
  CreateAttribute("filtering_scale_velunc",           1., WRITE_NOPUBLIC);
  CreateAttribute("filtering_scale_dirunc",           1., WRITE_NOPUBLIC);
  CreateAttribute("filtering_target_posunc",     1.25e-2, WRITE_NOPUBLIC);
  CreateAttribute("filtering_target_velunc",     1.25e-2, WRITE_NOPUBLIC);
  CreateAttribute("filtering_target_dirunc",     1.25e-2, WRITE_NOPUBLIC);
  CreateAttribute("filtering_user_unc",             5e-2, WRITE_NOPUBLIC);
  CreateAttribute("filtering_user_scale",            20., WRITE_NOPUBLIC);

  CreateAttribute("filtering_near_dist",               1, WRITE_NOPUBLIC);
  CreateAttribute("filtering_far_dist",              500, WRITE_NOPUBLIC);
  CreateAttribute("filtering_position_smooth_min",   0.3, WRITE_NOPUBLIC);
  CreateAttribute("filtering_position_smooth_max", 0.015, WRITE_NOPUBLIC);
  CreateAttribute("filtering_position_adapt_rate",  1E-7, WRITE_NOPUBLIC);
  CreateAttribute("filtering_direction_smooth_min", 0.025, WRITE_NOPUBLIC);
  CreateAttribute("filtering_direction_smooth_max", 0.0001, WRITE_NOPUBLIC);
  CreateAttribute("filtering_direction_adapt_rate", 0.025, WRITE_NOPUBLIC);

  CreateAttribute("tracking_classify_threshold",     0.74757, WRITE_NOPUBLIC);
  CreateAttribute("tracking_transform_angle",          0, WRITE_ALWAYS);
  CreateAttribute("tracking_shift_x",                  0, WRITE_ALWAYS);

  CreateAttribute("holotopic_tomography_slices",     0.9, WRITE_NOPUBLIC);

  CreateAttribute("image_processing_auto_flip",     true, WRITE_ALWAYS);
  CreateAttribute("image_processing_cropped",      false, WRITE_NOPUBLIC);
  CreateAttribute("image_processing_throttled",    false, WRITE_NOPUBLIC);
  CreateAttribute("image_processing_standby",       true, WRITE_NOPUBLIC);
  CreateAttribute("image_processing_mode",             1, WRITE_NOPUBLIC);

  CreateAttribute("power_saving_adapter",          false, WRITE_ALWAYS);
  CreateAttribute("power_saving_battery",           true, WRITE_ALWAYS);

  CreateAttribute("use_robust_tips",                true, WRITE_NOPUBLIC);

  CreateAttribute("os_interaction_mode", Leap::OutputPeripheral::OUTPUT_MODE_DISABLED, WRITE_ALWAYS);
  CreateAttribute("os_interaction_multi_monitor",  false, WRITE_ALWAYS);

  CreateAttribute("interaction_box_auto",          false, WRITE_ALWAYS);
  CreateAttribute("interaction_box_height",          200, WRITE_ALWAYS);
  CreateAttribute("interaction_box_scale",           0.8, WRITE_ALWAYS);

  CreateAttribute("interaction_center_x",              0, WRITE_ALWAYS);
  CreateAttribute("interaction_center_y",            200, WRITE_ALWAYS);
  CreateAttribute("interaction_center_z",              0, WRITE_ALWAYS);

  CreateAttribute("autoplane_closeness_epsilon",     4.0, WRITE_ALWAYS);
  CreateAttribute("autoplane_covariance_scale_factor", 0.5, WRITE_ALWAYS);
  CreateAttribute("autoplane_time_window",           0.4, WRITE_ALWAYS);
  CreateAttribute("autoplane_warmup_time",         0.333, WRITE_ALWAYS);
  CreateAttribute("autoplane_hover_zone_cutoff",    70.0, WRITE_ALWAYS);
  CreateAttribute("autoplane_dead_zone_cutoff",    220.0, WRITE_ALWAYS);
  CreateAttribute("autoplane_function_steepness",    1.0, WRITE_ALWAYS);
  CreateAttribute("autoplane_function_width",        4.0, WRITE_ALWAYS);
  CreateAttribute("autoplane_function_speed",        0.5, WRITE_ALWAYS);

  CreateAttribute("klaatu_barada_nikto",            true, WRITE_ALWAYS);

  CreateAttribute("FPS_throttle",                   true,WRITE_NOPUBLIC);

  CreateAttribute("adjust_radiance", true,WRITE_NOPUBLIC);
  // Hand, finger, and tool isolation to remove heads, etc. from frame
  CreateAttribute("hand_isolation_enabled",         true, WRITE_ALWAYS);

  CreateAttribute("screen_detected",        false, WRITE_ALWAYS);
  CreateAttribute("screen_point0_x",        0., WRITE_ALWAYS);
  CreateAttribute("screen_point0_y",        0., WRITE_ALWAYS);
  CreateAttribute("screen_point0_z",        0., WRITE_ALWAYS);
  CreateAttribute("screen_point1_x",        0., WRITE_ALWAYS);
  CreateAttribute("screen_point1_y",        0., WRITE_ALWAYS);
  CreateAttribute("screen_point1_z",        0., WRITE_ALWAYS);
  CreateAttribute("screen_point2_x",        0., WRITE_ALWAYS);
  CreateAttribute("screen_point2_y",        0., WRITE_ALWAYS);
  CreateAttribute("screen_point2_z",        0., WRITE_ALWAYS);
  CreateAttribute("screen_point3_x",        0., WRITE_ALWAYS);
  CreateAttribute("screen_point3_y",        0., WRITE_ALWAYS);
  CreateAttribute("screen_point3_z",        0., WRITE_ALWAYS);
  CreateAttribute("screen_point4_x",        0., WRITE_ALWAYS);
  CreateAttribute("screen_point4_y",        0., WRITE_ALWAYS);
  CreateAttribute("screen_point4_z",        0., WRITE_ALWAYS);
  CreateAttribute("screen_point5_x",        0., WRITE_ALWAYS);
  CreateAttribute("screen_point5_y",        0., WRITE_ALWAYS);
  CreateAttribute("screen_point5_z",        0., WRITE_ALWAYS);
  CreateAttribute("screen_point6_x",        0., WRITE_ALWAYS);
  CreateAttribute("screen_point6_y",        0., WRITE_ALWAYS);
  CreateAttribute("screen_point6_z",        0., WRITE_ALWAYS);
  CreateAttribute("screen_point7_x",        0., WRITE_ALWAYS);
  CreateAttribute("screen_point7_y",        0., WRITE_ALWAYS);
  CreateAttribute("screen_point7_z",        0., WRITE_ALWAYS);

  CreateAttribute("reflection_removal_enabled", true, WRITE_ALWAYS);
#if IS_INTERNAL_BUILD == 1
  CreateAttribute("auto_check_updates", false, WRITE_ALWAYS);
#else
  CreateAttribute("auto_check_updates", true, WRITE_ALWAYS);
#endif
  CreateAttribute("force_accept_updates", false, WRITE_ALWAYS);

  for (int i=0; i<MAX_NUM_CALIBRATED_SCREENS; i++) {
    std::stringstream ss;
    Value::Array array;
    if (i == 0) { // only use one default screen, the rest are empty
      std::vector<double>& defaultCalib = GetDefaultScreenCalib();
      for (size_t j=0; j<defaultCalib.size(); j++) {
        array.push_back(defaultCalib[j]);
      }
    }
    ss << "screen_calibration" << i;
    CreateAttribute(ss.str(),                        array, WRITE_ALWAYS);
  }

#ifdef LANGUAGE_MENU
  CreateAttribute("display_language", "", WRITE_ALWAYS);
#endif
  LoadImageConfig();
}

void Config::CreateAttribute(const std::string& attributeName, const Value& value, AccessType accessType)
{
  static ConfigState& s = state();
  boost::unique_lock<boost::recursive_mutex> lock(s.MapMutex);
  Attribute temp;
  temp.value = value;
  temp.accessType = accessType;
  s.AttributeMap[attributeName] = temp;
}

bool Config::SetAttribute(const std::string& attributeName, const Value& value, bool userSpecified)
{
  static ConfigState& s = state();
  boost::unique_lock<boost::recursive_mutex> lock(s.MapMutex);
  std::map<std::string, Attribute>::iterator found = s.AttributeMap.find(attributeName);
  if (found == s.AttributeMap.end()) {
    std::map<std::string, std::shared_ptr<DynamicAttribute> >::iterator dynamicFound =
      s.DynamicAttributeMap.find(attributeName);
    if (dynamicFound != s.DynamicAttributeMap.end()) {
      bool wasSet = dynamicFound->second->Setter(value);
      if (wasSet) {
        NotifyClients(attributeName, value);
      }
      return wasSet;
    }
    return false;
  }
  const int curAccessType = found->second.accessType;
  if (curAccessType == READ_ONLY) {
    return false;
  }
  if (IS_INTERNAL_BUILD != 1 && curAccessType == WRITE_NOPUBLIC && userSpecified) {
    return false;
  }
  if (found->second.value != value) {
    found->second.value = value;
    if (userSpecified) {
      s.ModifiedMap[attributeName] = value;
    }
    // Tell registered clients that an attribute has changed
    NotifyClients(attributeName, value);
  }
  return true;
}

// bool Config::RegisterDynamicAttribute(const std::string& attributeName,
//                                       const std::shared_ptr<DynamicAttribute>& dynamicAttribute)
// {
//   static ConfigState& s = state();
//   boost::unique_lock<boost::recursive_mutex> lock(s.MapMutex);
//   s.DynamicAttributeMap.find(attributeName);
//   if (dynamicAttribute &&
//       s.DynamicAttributeMap.find(attributeName) == s.DynamicAttributeMap.end() &&
//       s.AttributeMap.find(attributeName) == s.AttributeMap.end()) {
//     s.DynamicAttributeMap[attributeName] = dynamicAttribute;
//
//     NotifyClients(attributeName, dynamicAttribute->Getter());
//
//     return true;
//   }
//   return false;
// }
//
// bool Config::UnregisterDynamicAttribute(const std::string& attributeName)
// {
//   static ConfigState& s = state();
//   boost::unique_lock<boost::recursive_mutex> lock(s.MapMutex);
//   std::map<std::string, std::shared_ptr<DynamicAttribute> >::iterator found = s.DynamicAttributeMap.find(attributeName);
//   if (found != s.DynamicAttributeMap.end()) {
//     s.DynamicAttributeMap.erase(found);
//     NotifyClients(attributeName, Value());
//     return true;
//   }
//   return false;
// }

void Config::LoadFromFile(const std::string& fileName, const std::string& section) {
  Value object;
  {
    std::ifstream in(fileName.c_str());
    SetOutputFile(fileName, section);
    if(!in)
      throw_rethrowable std::runtime_error("Configuration file " + fileName + " not found");

    in >> object;
  }

  if(!object.IsHash())
    throw_rethrowable std::runtime_error("Error parsing config file: " + fileName);

  Value config = object.HashGet(section);
  if(!config.IsHash())
    throw_rethrowable std::runtime_error("Section '" + section + "' not found: " + fileName);

  const Value::Hash& hash = config.ConstCast<Value::Hash>();

  for (Value::Hash::const_iterator iter = hash.begin(); iter != hash.end(); ++iter)
    SetAttribute(iter->first, iter->second, true);

  if(!LoadImageConfig())
    throw_rethrowable std::runtime_error("Error loading image config");
}

bool Config::LoadFromFile(const std::string& fileName, bool verbose, const std::string& section) {
  try {
    LoadFromFile(fileName, section);
  } catch(std::runtime_error& ex) {
    if(verbose)
      std::cout << ex.what();
    return false;
  }
  return true;
}

void Config::SetOutputFile(const std::string& fileName, const std::string& section)
{
  static ConfigState& s = state();

  boost::unique_lock<boost::recursive_mutex> lock(s.MapMutex);

  if (s.SectionToFileMap.empty()) {
    s.SectionToFileMap[""] = fileName; // Set entry for default section
  }
  s.SectionToFileMap[section] = fileName;
}

bool Config::Save(const std::string& section, bool toDefault)
{
  static ConfigState& s = state();
  bool status = false;

  boost::unique_lock<boost::recursive_mutex> lock(s.MapMutex);

  if (!s.ModifiedMap.empty() || toDefault) {
    std::map<std::string, std::string>::const_iterator found = s.SectionToFileMap.find(section);
    if (found == s.SectionToFileMap.end()) {
      found = s.SectionToFileMap.find(""); // Use default file name
    }
    if (found != s.SectionToFileMap.end()) {
      const std::string fileName = found->second;

      try {
        Value object;

        std::ifstream in(fileName.c_str(), std::ios::in | std::ios::binary);

        if (in.is_open()) {
          in >> object;
          in.close();
        }

        if (!object.IsHash()) {
          object = Value::Hash();
        }
        std::string tmpFileName = fileName + ".tmp";
        std::ofstream out(tmpFileName.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);

        if (out.is_open()) {
          if (toDefault) {
            std::map<std::string, Attribute> emptyMap;
            object.HashSet(section, emptyMap);
          } else {
            object.HashSet(section, s.ModifiedMap);
          }
          out << object.ToJSON(false, true);
          out.close();
          status = true;
          boost::filesystem::rename(tmpFileName, fileName);
        }
        boost::filesystem::remove(tmpFileName);
      } catch (...) {
        status = false;
      }
    }
  }
  return status;
}

// void Config::RegisterOnChange(const std::string& name,
//                               const boost::function<void(const std::string&, const Value&)>& function)
// {
//   static ConfigState& s = state();
//
//   boost::unique_lock<boost::recursive_mutex> lock(s.MapMutex);
//   s.OnChangeFunctions[name] = function;
// }
//
// void Config::UnregisterOnChange(const std::string& name)
// {
//   static ConfigState& s = state();
//
//   boost::unique_lock<boost::recursive_mutex> lock(s.MapMutex);
//   s.OnChangeFunctions.erase(name);
// }

void Config::NotifyClients( const std::string& attributeName, const Value& value ) {
  static ConfigState& s = state();

  // Tell registered clients that an attribute has changed
  for(
    ConfigState::t_mpChangeFunc::const_iterator iter = s.OnChangeFunctions.begin();
    iter != s.OnChangeFunctions.end();
    ++iter
  )
  {
    iter->second(attributeName, value);
  }
}

bool Config::LoadImageConfig()
{
  static ConfigState& s = state();
  std::string mode;
  if (Config::GetAttribute<std::string>("camera_mode", mode)) {
    if (!mode.compare("VGA")) {
      s.Width = s.CalibWidth = s.SourceWidth = 640;
      s.Height = s.CalibHeight = s.SourceHeight = 480;
      s.DownSampleRate = 1;
      return true;
    } else if (!mode.compare("HVGA")) {
      s.Width = s.CalibWidth = s.SourceWidth = 640;
      s.CalibHeight = 480;
      s.Height = s.SourceHeight = 240;
      s.DownSampleRate = 2;
      return true;
    } else if (!mode.compare("QVGA")) {
      s.Width = s.SourceWidth = 320;
      s.Height = s.SourceHeight = 240;
      s.CalibWidth = 640;
      s.CalibHeight = 480;
      s.DownSampleRate = 2;
      return true;
    } else if (!mode.compare("HHVGA")) {
      s.Width = s.CalibWidth = s.SourceWidth = 640;
      s.CalibHeight = 480;
      s.Height = s.SourceHeight = 120;
      s.DownSampleRate = 4;
      return true;
    } else if (!mode.compare("QHVGA")) {
      s.CalibWidth = 640;
      s.CalibHeight = 480;
      s.Width = s.SourceWidth = 320;
      s.Height = s.SourceHeight = 120;
      s.DownSampleRate = 4;
      return true;
    }
  }
  return
    Config::GetAttribute<int>("image_width", s.Width) &&
    Config::GetAttribute<int>("image_height", s.Height) &&
    Config::GetAttribute<int>("image_calib_width", s.CalibWidth) &&
    Config::GetAttribute<int>("image_calib_height", s.CalibHeight) &&
    Config::GetAttribute<int>("image_source_width", s.SourceWidth) &&
    Config::GetAttribute<int>("image_source_height", s.SourceHeight);
}

// void Config::GetImageConfig(int& width, int& height, int& downsampleRate)
// {
//   static ConfigState& s = state();
//   width = s.Width;
//   height = s.Height;
//   downsampleRate = s.DownSampleRate;
// }
//
// void Config::SetImageConfig(int  width, int  height, int  downsampleRate)
// {
//   static ConfigState& s = state();
//   s.Width = width;
//   s.Height = height;
//   s.DownSampleRate = downsampleRate;
// }
//
// void Config::GetCalibImageConfig(int& calibWidth, int& calibHeight)
// {
//   static ConfigState& s = state();
//   calibWidth = s.CalibWidth;
//   calibHeight = s.CalibHeight;
// }
//
// void Config::SetCalibImageConfig(int  calibWidth, int  calibHeight)
// {
//   static ConfigState& s = state();
//   s.CalibWidth = calibWidth;
//   s.CalibHeight = calibHeight;
// }
//
// void Config::GetSourceImageConfig(int& srcWidth, int& srcHeight)
// {
//   static ConfigState& s = state();
//   srcWidth = s.SourceWidth;
//   srcHeight = s.SourceHeight;
// }
//
// void Config::SetSourceImageConfig(int  srcWidth, int  srcHeight)
// {
//   static ConfigState& s = state();
//   s.SourceWidth = srcWidth;
//   s.SourceHeight = srcHeight;
// }
//
// void Config::SetCameraMode(CameraMode mode, bool userSpecified)
// {
//   switch(mode) {
//     case VGA:
//       Config::SetAttribute("camera_mode", "VGA", userSpecified);
//       break;
//     case HVGA:
//       Config::SetAttribute("camera_mode", "HVGA", userSpecified);
//       break;
//     case QVGA:
//       Config::SetAttribute("camera_mode", "QVGA", userSpecified);
//       break;
//     case HHVGA:
//       Config::SetAttribute("camera_mode", "HHVGA", userSpecified);
//       break;
//     case QHVGA:
//       Config::SetAttribute("camera_mode", "QHVGA", userSpecified);
//       break;
//     default:
//       Config::SetAttribute("camera_mode", "HVGA", userSpecified);
//       break;
//   }
//   if (userSpecified) {
//     Config::Save();
//   }
//   LoadImageConfig();
// }
//
// Config::CameraMode Config::GetCameraMode()
// {
//   std::string mode;
//   Config::GetAttribute<std::string>("camera_mode", mode);
//   if (!mode.compare("VGA"))    {return Config::VGA;}
//   if (!mode.compare("HVGA"))   {return Config::HVGA;}
//   if (!mode.compare("QVGA"))   {return Config::QVGA;}
//   if (!mode.compare("HHVGA"))  {return Config::HHVGA;}
//   if (!mode.compare("QHVGA"))  {return Config::QHVGA;}
//   return Config::HVGA;
// }

std::vector<double>& Config::GetDefaultScreenCalib() {
  static std::vector<double> calib;
  if (calib.empty()) {
    const size_t valLen = 9;
    double vals[valLen] = {
      -200,  50, -200,
       400,   0,    0,
         0, 250,    0
    };
    calib.assign(vals, vals + valLen);
  }
  return calib;
}
