import copy

# Get instancemethod type. Is there a faster way?
class Foo():
    def method(self):
        pass
type_instancemethod = type(Foo().method)

# Makes get_ methods accessible as read only properties. o.get_foo() can be
# accessed as o.foo. Useful for computed properties.

class PropWrap(object):
    def __getattr__(self, name):
        # require read-only computed properties to start with get_
        try:
            v = object.__getattribute__(self, 'get_' + name)
            if type(v) == type_instancemethod:
                v = v()
            return v
        except:
            raise

# Makes a dict accessible by property syntax. d[key] can be accessed as
# d.key. Derives from PropWrap, for computed properties.

class DictWrap(PropWrap):
    def __init__(self, d):
        if isinstance(d, DictWrap):
            d = d.__dict__
        self.__dict__ = copy.deepcopy(d)
        for key in self.__dict__.keys():
            if isinstance(self.__dict__[key], dict):
                self.__dict__[key] = DictWrap(self.__dict__[key])
            if isinstance(self.__dict__[key], list):
                for i in xrange(len(self.__dict__[key])):
                    if isinstance(self.__dict__[key][i], dict):
                        self.__dict__[key][i] = DictWrap(self.__dict__[key][i])

    def __repr__(self):
        return self.__dict__.__repr__()

# Wraps an object, so that properties on that object are accessible via
# this wrapped object, and adds computed properties. Useful for wrapping
# db.Model objects, for example.

class ObjWrap(PropWrap):
    def __init__(self, obj):
        self.obj = obj

    def __getattr__(self, name):
        try:
            return super(ObjWrap, self).__getattr__(name)
        except:
            pass
        return getattr(self.obj, name)

