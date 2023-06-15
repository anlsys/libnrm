NRM Configuration/Manifest Guide
================================

The ``dhall`` and ``dhall-to-json`` utilities are available as convenience in
this environment should you need them.

Types common to both daemon configuration and manifest scripts can be found here:

.. literalinclude:: ../../examples/common_types.dhall
   :caption: /examples/common_types.dhall
   :linenos:

Daemon Configuration
--------------------

Types
^^^^^

``nrmd``'s configuration can be defined
in ``json``, ``yaml``, or `Dhall`_ formats. Admissible values are defined
via the following ``dhall`` file:

.. literalinclude:: ../../examples/nrmd/nrmd_types.dhall
   :caption: /examples/nrmd/nrmd_types.dhall
   :linenos:

Defaults
^^^^^^^^

Optional values are filled using defaults, here expressed in
both ``dhall`` and ``json``:

.. literalinclude:: ../../examples/nrmd/nrmd_defaults.dhall
   :caption: /examples/nrmd/nrmd_defaults.dhall
   :linenos:

.. literalinclude:: ../../examples/nrmd/nrmd_defaults.json
   :caption: /examples/nrmd/nrmd_defaults.json
   :linenos:

Examples
^^^^^^^^

Attribute merging is always performed with these defaults. Example configurations
are located in the `examples`_ folder, but we include each of them here. Note that
Dhall is valid as a configuration language:

.. literalinclude:: ../../examples/nrmd/control.yaml
   :caption: /examples/nrmd/control.yaml
   :linenos:

.. literalinclude:: ../../examples/nrmd/control.dhall
   :caption: /examples/nrmd/control.dhall
   :linenos:

.. literalinclude:: ../../examples/nrmd/extra-static-sensor.dhall
   :caption: /examples/nrmd/extra-static-sensor.dhall
   :linenos:

.. literalinclude:: ../../examples/nrmd/extra-static-sensor.yaml
   :caption: /examples/nrmd/extra-static-sensor.yaml
   :linenos:

.. literalinclude:: ../../examples/nrmd/extra-static-actuator.json
   :caption: /examples/nrmd/extra-static-actuator.json
   :linenos:

Manifest Configuration
----------------------

Types
^^^^^

Manifest files can also be defined either using Dhall, ``yaml``, or ``json``.
Admissible values are defined via the following ``dhall`` file:

.. literalinclude:: ../../examples/manifests/manifest_types.dhall
   :caption: /examples/manifests/manifest_types.dhall
   :linenos:

Defaults
^^^^^^^^

Under-specified manifests are also filled with defaults, specified here
in both ``dhall`` and ``json``:

.. literalinclude:: ../../examples/manifests/manifest_defaults.dhall
   :caption: /examples/manifests/manifest_defaults.dhall
   :linenos:

.. literalinclude:: ../../examples/manifests/manifest_defaults.json
   :caption: /examples/manifests/manifest_defaults.json
   :linenos:

Examples
^^^^^^^^

The following could be used with the `libnrm`_ interface:

.. literalinclude:: ../../examples/manifests/libnrm.dhall
   :caption: /examples/manifests/libnrm.dhall
   :linenos:

.. literalinclude:: ../../examples/manifests/libnrm.json
   :caption: /examples/manifests/libnrm.json
   :linenos:

The following could be used with the `perf`_ utility:

.. literalinclude:: ../../examples/manifests/perfwrap.dhall
   :caption: /examples/manifests/perfwrap.dhall
   :linenos:

.. literalinclude:: ../../examples/manifests/perfwrap.yaml
   :caption: /examples/manifests/perfwrap.yaml
   :linenos:

.. _`Dhall`: https://dhall-lang.org/
.. _`examples`: https://github.com/anlsys/nrm-docs/tree/main/examples
.. _`libnrm`: https://github.com/anlsys/libnrm
.. _`perf`: https://www.man7.org/linux/man-pages/man1/perf.1.html
