Recycle Documentation
=====================

Recycle is an implementation of a simple C++ resource pool.

The ``recycle`` project contains two types of resource pools:

1. The ``recycle::shared_pool`` is useful when managing expensive to
   construct objects. The life-time of the managed objects is controlled
   by using ``std::shared_ptr``. A custom deleter is used to reclaim
   objects in the pool when the last remaining ``std::shared_ptr`` owning
   the object is destroyed.

2. The ``recycle::unique_pool`` works the same way as the
   ``recycle::shared_pool`` but instead uses ``std::unique_ptr`` for
   managing the resources. Still we need a custom deleter - with
   ``std::unique_ptr`` this has to be part of the type. So the
   ``std::unique_ptr`` returned by ``recycle::unique_pool`` is
   of type ``recycle::unique_pool::pool_ptr``.

Besides the fact that ``recycle::shared_pool`` manages ``std::shared_ptr`` and
``recycle::unique_pool`` manages ``std::unique_ptr`` the API should be the
same. So in the following you can replace ``shared`` with ``unique`` to
swap the behavior.


Table of Contents
-----------------

.. toctree::
  :maxdepth: 2

  user_api/user_api
  license


.. toctree::
  :maxdepth: 1

  news
