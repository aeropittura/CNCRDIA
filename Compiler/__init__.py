"""Historical name of the :mod:`mpcl` package.

Programs and scripts written for the original layout import modules such
as ``Compiler.types`` or ``Compiler.library``.  Every such name is an alias
of the module in its new location (see :data:`mpcl.legacy.MODULES`), so
``Compiler.types.sint is mpcl.core.types.sint`` holds and existing code
keeps working unchanged.
"""

import importlib
import importlib.abc
import importlib.util
import sys

from mpcl.legacy import MODULES as _MODULES


class _LegacyModuleFinder(importlib.abc.MetaPathFinder, importlib.abc.Loader):
    """Resolve ``Compiler.<name>`` to the already existing new module."""

    def find_spec(self, fullname, path=None, target=None):
        if fullname in _MODULES:
            return importlib.util.spec_from_loader(fullname, self)
        return None

    def create_module(self, spec):
        return importlib.import_module(_MODULES[spec.name])

    def exec_module(self, module):
        pass


if not any(isinstance(f, _LegacyModuleFinder) for f in sys.meta_path):
    sys.meta_path.insert(0, _LegacyModuleFinder())

from mpcl import *  # noqa: E402,F401,F403  (the old package namespace)


def __getattr__(name):
    """``import Compiler; Compiler.ml`` loads the aliased module lazily."""
    legacy = __name__ + '.' + name
    if legacy in _MODULES:
        return importlib.import_module(legacy)
    raise AttributeError('module %r has no attribute %r' % (__name__, name))
