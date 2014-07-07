/// @file wi.cc
/// This example demonstrates loading, running and scripting a very simple NaCl
/// module.  To load the NaCl module, the browser first looks for the
/// CreateModule() factory method (at the end of this file).  It calls
/// CreateModule() once to load the module code from your .nexe.  After the
/// .nexe code is loaded, CreateModule() is not called again.
///
/// Once the .nexe code is loaded, the browser than calls the CreateInstance()
/// method on the object returned by CreateModule().  It calls CreateInstance()
/// each time it encounters an <embed> tag that references your NaCl module.
///
/// When the browser encounters JavaScript that references your NaCl module, it
/// calls the GetInstanceObject() method on the object returned from
/// CreateInstance().  In this example, the returned object is a subclass of
/// ScriptableObject, which handles the scripting support.

#include <ppapi/cpp/dev/scriptable_object_deprecated.h>
#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/module.h>
#include <ppapi/cpp/var.h>
#include <cstdio>
#include <string>

/// These are the method names as JavaScript sees them.  Add any methods for
/// your class here.
namespace {
// A method consists of a const char* for the method ID and the method's
// declaration and implementation.
// TODO(sdk_user): 1. Add the declarations of your method IDs.

// TODO(sdk_user): 2. Implement the methods that correspond to your method IDs.
}  // namespace

// Note to the user: This glue code reflects the current state of affairs.  It
// may change.  In particular, interface elements marked as deprecated will
// disappear sometime in the near future and replaced with more elegant
// interfaces.  As of the time of this writing, the new interfaces are not
// available so we have to provide this code as it is written below.

/// This class exposes the scripting interface for this NaCl module.  The
/// HasMethod method is called by the browser when executing a method call on
/// the object.  The name of the JavaScript function (e.g. "fortyTwo") is
/// passed in the |method| paramter as a string pp::Var.  If HasMethod()
/// returns |true|, then the browser will call the Call() method to actually
/// invoke the method.
class WiScriptableObject : public pp::deprecated::ScriptableObject {
 public:
  /// Called by the browser to decide whether @a method is provided by this
  /// plugin's scriptable interface.
  /// @param[in] method The name of the method
  /// @param[out] exception A pointer to an exception.  May be used to notify
  ///     the browser if an exception occurs.
  /// @return true iff @a method is one of the exposed method names.
  virtual bool HasMethod(const pp::Var& method, pp::Var* exception);

  /// Invoke the function associated with @a method.  The argument list passed
  /// in via JavaScript is marshalled into a vector of pp::Vars.  None of the
  /// functions in this example take arguments, so this vector is always empty.
  /// @param[in] method The name of the method to be invoked.
  /// @param[in] args The arguments to be passed to the method.
  /// @param[out] exception A pointer to an exception.  May be used to notify
  ///     the browser if an exception occurs.
  /// @return true iff @a method was called successfully.
  virtual pp::Var Call(const pp::Var& method,
                       const std::vector<pp::Var>& args,
                       pp::Var* exception);
};

bool WiScriptableObject::HasMethod(const pp::Var& method,
                                             pp::Var* exception) {
  if (!method.is_string()) {
    return false;
  }
  std::string method_name = method.AsString();
  // TODO(sdk_user): 3. Make this function return true iff method_name is equal
  // to any of your method IDs.
  bool has_method = false;
  return has_method;
}

pp::Var WiScriptableObject::Call(const pp::Var& method,
                                           const std::vector<pp::Var>& args,
                                           pp::Var* exception) {
  if (!method.is_string()) {
    return pp::Var();
  }
  std::string method_name = method.AsString();
  // TODO(sdk_user): 4. Make this function call whatever method has method_name
  // as its method ID.
  return pp::Var();
}

/// The Instance class.  One of these exists for each instance of your NaCl
/// module on the web page.  The browser will ask the Module object to create
/// a new Instance for each occurence of the <embed> tag that has these
/// attributes:
///     type="application/x-nacl"
///     nexes="ARM: wi_arm.nexe
///            ..."
/// The Instance can return a ScriptableObject representing itself.  When the
/// browser encounters JavaScript that wants to access the Instance, it calls
/// the GetInstanceObject() method.  All the scripting work is done though
/// the returned ScriptableObject.
class WiInstance : public pp::Instance {
 public:
  /// The constructor creates the plugin-side instance.
  /// @param[in] instance the handle to the browser-side plugin instance.
  explicit WiInstance(PP_Instance instance) : pp::Instance(instance)
  {}
  virtual ~WiInstance() {}

  /// The browser calls this function to get a handle, in form of a pp::Var,
  /// to the plugin-side scriptable object.  The pp::Var takes over ownership
  /// of said scriptable, meaning the browser can call its destructor.  The
  /// WiScriptableObject is the plugin-side representation of that
  /// scriptable object.
  /// @return The browser's handle to the plugin side instance.
  virtual pp::Var GetInstanceObject() {
    WiScriptableObject* hw_object =
        new WiScriptableObject();
    return pp::Var(this, hw_object);
  }
};

/// The Module class.  The browser calls the CreateInstance() method to create
/// an instance of your NaCl module on the web page.  The browser creates a new
/// instance for each <embed> tag with type="application/x-nacl".
class WiModule : public pp::Module {
 public:
  WiModule() : pp::Module() {}
  virtual ~WiModule() {}

  /// Create and return a WiInstance object.
  /// @param[in] instance The browser-side instance.
  /// @return the plugin-side instance.
  virtual pp::Instance* CreateInstance(PP_Instance instance) {
    return new WiInstance(instance);
  }
};

namespace pp {
/// Factory function called by the browser when the module is first loaded.
/// The browser keeps a singleton of this module.  It calls the
/// CreateInstance() method on the object you return to make instances.  There
/// is one instance per <embed> tag on the page.  This is the main binding
/// point for your NaCl module with the browser.
Module* CreateModule() {
  return new WiModule();
}
}  // namespace pp
