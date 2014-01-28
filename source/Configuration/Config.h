/*==================================================================================================================

    Copyright (c) 2010 - 2012 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

/// <summary>
/// Configuration loading/storage class
/// </summary>
/// <remarks>
/// This class loads a saved configuration from an XML file and adds its attributes to an internal storage map.
/// Default values for the various values are initialized beforehand, allowing access restrictions to be set on
/// what can be changed from the XML file. Then attributes are retrieved by parsing the stored value in the map
/// into the desired type.
///
/// Maintainers: Raffi, Hua, Jonathan
/// </remarks>

#ifndef __Config_h__
#define __Config_h__

#include "common.h"
#include "DataStructures/Value.h"
#include <iostream>
#include <boost/thread.hpp>
#include SHARED_PTR_HEADER

class Config
{
public:
  enum AccessType { READ_ONLY, WRITE_NOPUBLIC, WRITE_ALWAYS };

  static void InitializeDefaults();
  static bool SetAttribute(const std::string& attributeName, const Value& value, bool userSpecified = false);
  static void SetOutputFile(const std::string& fileName, const std::string& section = "configuration");
  static bool Save(const std::string& section = "configuration", bool toDefault = false);
  static void RegisterOnChange(const std::string& name, const boost::function<void(const std::string&, const Value&)>& function);
  static void UnregisterOnChange(const std::string& name);

  static bool LoadFromFile(const std::string& fileName, bool verbose, const std::string& section = "configuration");

  /// <summary>
  /// Trivial loading routine.
  /// </summary>
  /// <throws>std::runtime_error in the event of a problem in processing</throws>
  static void LoadFromFile(const std::string& fileName, const std::string& section = "configuration");

  template <class T>
  static bool GetAttribute(const std::string& attributeName, T& data)
  {
    static ConfigState& s = state();
    // TODO:  Verify that unique_lock is what is wanted here.
    // unique_lock does not actually obtain the lock, it just allows a way to recursively acquire it.
    // If this isn't desired, use lock_guard instead.

    std::map<std::string, Attribute>::const_iterator found = s.AttributeMap.find(attributeName);
    if (found != s.AttributeMap.end()) {
      data = found->second.value.To<T>();
      return true;
    }
    std::map<std::string, std::shared_ptr<DynamicAttribute> >::const_iterator dynamicFound = s.DynamicAttributeMap.find(attributeName);
    if (dynamicFound != s.DynamicAttributeMap.end()) {
      data = dynamicFound->second->Getter().To<T>();
      return true;
    }
    return false;
  }

  static void GetAttributes(Value::Hash& attributes)
  {
    static ConfigState& s = state();
    for (std::map<std::string, Attribute>::const_iterator iter = s.AttributeMap.begin();
         iter != s.AttributeMap.end(); ++iter) {
      attributes[iter->first] = iter->second.value;
    }
    for (std::map<std::string, std::shared_ptr<DynamicAttribute> >::const_iterator iter =
         s.DynamicAttributeMap.begin(); iter != s.DynamicAttributeMap.end(); ++iter) {
      if (iter->second) {
        attributes[iter->first] = iter->second->Getter();
      }
    }
  }

  static void GetPublicAttributes(Value::Hash& attributes) {
    static ConfigState& s = state();
    for (std::map<std::string, Attribute>::const_iterator iter = s.AttributeMap.begin();
         iter != s.AttributeMap.end(); ++iter) {
      if (iter->second.accessType == WRITE_ALWAYS) {
        attributes[iter->first] = iter->second.value;
      }
    }
  }

  class DynamicAttribute
  {
  public:
    DynamicAttribute() {}
    virtual ~DynamicAttribute() {}

    virtual Value Getter() const = 0;
    virtual bool Setter(const Value& value) = 0;
  };

  template<typename T>
  class DynamicVariable : public DynamicAttribute
  {
  public:
    DynamicVariable(T& variable) : m_variable(variable) {}

    virtual Value Getter() const { return Value(m_variable); }
    virtual bool Setter(const Value& value) { m_variable = value.To<T>(); return true; }
  private:
    T& m_variable;
  };

  static bool RegisterDynamicAttribute(const std::string& attributeName,
                                       const std::shared_ptr<DynamicAttribute>& dynamicAttribute);
  template<typename T>
  static bool RegisterDynamicVariable(const std::string& attributeName, T& dynamicVariable)
  {
    return RegisterDynamicAttribute(attributeName,
                                    std::shared_ptr<DynamicAttribute>(new DynamicVariable<T>(dynamicVariable)));
  }
  static bool UnregisterDynamicAttribute(const std::string& attributeName);

  static void GetImageConfig(int& width, int& height, int& downsampleRate);
  static void SetImageConfig(int  width, int  height, int  downsampleRate);
  static void GetCalibImageConfig(int& calibWidth, int& calibHeight);
  static void SetCalibImageConfig(int  calibWidth, int  calibHeight);
  static void GetSourceImageConfig(int& srcWidth, int& srcHeight);
  static void SetSourceImageConfig(int  srcWidth, int  srcHeight);

  static const int MAX_NUM_CALIBRATED_SCREENS = 10;
  static std::vector<double>& GetDefaultScreenCalib();

  enum CameraMode {UNKNOWN, VGA, HVGA, QVGA, HHVGA, QHVGA, ROBUSTMODE, NUM_CAMERA_MODES};
  static void SetCameraMode(CameraMode mode, bool userSpecified = false);
  static CameraMode GetCameraMode();

private:

  static void NotifyClients( const std::string& attributeName, const Value& value );

  static bool LoadImageConfig();
  static void CreateAttribute(const std::string& attributeName, const Value& value, AccessType accessType);

  struct Attribute
  {
    Value value;
    AccessType accessType;
  };

  struct ConfigState {
    typedef std::map<std::string, boost::function<void(const std::string&, const Value&)> > t_mpChangeFunc;

    std::map<std::string, Attribute> AttributeMap;
    std::map<std::string, std::shared_ptr<Config::DynamicAttribute> > DynamicAttributeMap;
    Value::Hash ModifiedMap;
    std::map<std::string, std::string> SectionToFileMap;
    t_mpChangeFunc OnChangeFunctions;
    boost::recursive_mutex MapMutex;
    int Width;
    int Height;
    int SourceWidth;
    int SourceHeight;
    int CalibWidth;
    int CalibHeight;
    int DownSampleRate;
  };

  static struct ConfigState& state();
};

#if IS_INTERNAL_BUILD == 1
  // used to expose an existing var for tweaking on client side.
  // exposed values are automatically detected and added to the tweak bar.
  #define TWEAK_VAR(NAME) \
    static bool bReg_##NAME = Config::RegisterDynamicVariable("tw_"#NAME, NAME); (void) bReg_##NAME

  // used to expose an existing var for tweaking on client side with an alternate name.
  // tweak vars need unique names. allows for disambiguation.
  // exposed values are automatically detected and added to the tweak bar.
  #define TWEAK_VAR_NAMED(VARNAME, TWEAKNAME) \
    static bool bReg_##TWEAKNAME = Config::RegisterDynamicVariable("tw_"#TWEAKNAME, VARNAME); (void) bReg_##NAME

  // used to declare a new var only used for exposure to tweaking.
  // TYPE is standard C++ data type (double, int32, bool specifically supported on client side)
  // NAME is C++ variable name - will be used as variable string name - make it unique.
  // INITVAL is intitial value of variable.
  #define TWEAK_DECL_VAR(TYPE, NAME, INITVAL)  \
    static TYPE NAME = INITVAL;                     \
    static bool bReg_##NAME = Config::RegisterDynamicVariable("tw_"#NAME, NAME); (void) bReg_##NAME

  // use this macro if you change the value on the host side and want the client tweak bar to see the change
  #define TWEAK_UPDATE_VAR(NAME) \
    Config::SetAttribute( "tw_"#NAME, Value(NAME) )

  // use this macro if you change the value on the host side and want the client tweak bar to see the change
  #define TWEAK_UPDATE_VAR_NAMED(VARNAME, TWEAKNAME) \
    Config::SetAttribute( "tw_"#TWEAKNAME, Value(VARNAME) )

  // only needed when a tweakable variable is going out of scope.
  // client side will get the unset message and remove from tweak bar.
  #define TWEAK_REMOVE_VAR(TWEAKNAME) \
    Config::UnregisterDynamicAttribute("tw_"#TWEAKNAME)

#else
  #define TWEAK_VAR(...)
  #define TWEAK_VAR_NAMED(...)
  #define TWEAK_DECL_VAR(...)
  #define TWEAK_UPDATE_VAR(...)
  #define TWEAK_UPDATE_VAR_NAMED(...)
  #define TWEAK_REMOVE_VAR(...)
#endif

#endif
