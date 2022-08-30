Using libnrm in your C applications
===================================

See the :doc:`Complete C API<doxy_c_api>` for more information.

1. Initialize NRM and create a client
-------------------------------------

To make code report progress to NRM, we need to:

- Include the library in the same way you include your other dependencies::

    # include <nrm.h>

- Declare :ref:`nrm_client<clients>`, :ref:`nrm_scope<scopes>`, and :ref:`nrm_sensor<sensors>` structures::

    static nrm_client_t *client;
    static nrm_scope_t *scope;
    static nrm_sensor_t *sensor;

- Initialize NRM, create a client for connecting to ``nrmd``::

    nrm_init(NULL, NULL);

    static char *upstream_uri = "tcp://127.0.0.1";
    static int pub_port = 2345;
    static int rpc_port = 3456;

    nrm_client_create(&client, upstream_uri, pub_port, rpc_port);


2. Determine measurements, create scopes and sensors
----------------------------------------------------

- Create a ``nrm_scope`` structure **for each** measurement to report, and add them to the client. If only reporting
  a single measurement::

    scope = nrm_scope_create();
    nrm_client_add_scope(client, scope);

  However, if reporting multiple measurements, create an array of scopes, e.g.::

    nrm_scope_t *nrm_scopes[NUM_MEASUREMENTS];
    for (int i=0; i<NUM_MEASUREMENTS, i++){
      scope = nrm_scope_create();
      nrm_scopes[i] = scope;
      nrm_client_add_scope(client, scope);
    }

- Configure your scope(s) to contain a resource type and corresponding logical indexes for that resource::

    // For example, for each logical CPU (calculated separately)
    for (int i=0; i<num_logical_cpus; i++) {
      nrm_scope_add(scope, NRM_SCOPE_TYPE_CPU, i);
    }

Other scope types include ``NRM_SCOPE_TYPE_NUMA`` and ``NRM_SCOPE_TYPE_GPU``.

- Create a sensor structure with a descriptive name, add it to the client::

    char *name = "cpu-measure";
    sensor = nrm_sensor_create(name);
    nrm_client_add_sensor(client, sensor);

3. Get time values, send measurements
-------------------------------------

- Report measurement progress to NRM using an NRM client, timestamp, sensor, and scope::

    nrm_time_t timestamp;
    int measurement;

    measurement = example_get_cpu_measurement();
    nrm_time_gettime(&timestamp);

    nrm_client_send_event(client, timestamp, sensor, scope, measurement);

4. Close down
-------------

- Delete the NRM scope(s), sensor, and client, finalize, and exit::

    nrm_scope_destroy(scope);
    nrm_sensor_destroy(&sensor);
    nrm_client_destroy(&client);
    nrm_finalize();
    exit(EXIT_SUCCESS);

5. Complete example
-------------------

The best way to understand how to make your application report progress to NRM is to use an example.
The following simple program uses each of the previously described NRM components
to periodically report a measurement that corresponds to a set of logical CPUs on some system. We also
assume that the NRM daemon ``nrmd`` is currently running on this system.::

   # include <stdio.h>
   # include <unistd.h>
   # include <nrm.h>

   int main()
   {
     int i, num_logical_cpus, measurement;

     static nrm_client_t *client;
     static nrm_scope_t *scope;
     static nrm_sensor_t *sensor;
     nrm_time_t timestamp;

     static char *upstream_uri = "tcp://127.0.0.1";
     static int pub_port = 2345;
     static int rpc_port = 3456;
     char *sensor_name = "example-measure";

     nrm_init(NULL, NULL);

     nrm_client_create(&client, upstream_uri, pub_port, rpc_port);

     scope = nrm_scope_create();
	   sensor = nrm_sensor_create(sensor_name);

     nrm_client_add_scope(client, scope);
     nrm_client_add_sensor(client, sensor);

     num_logical_cpus = example_get_num_logical_cpus();
     for (int i=0; i<num_logical_cpus; i++) {
       nrm_scope_add(scope, NRM_SCOPE_TYPE_CPU, i);
     }

     printf("hello\n")

     do {
       measurement = example_get_cpu_measurement();
       nrm_time_gettime(&timestamp);
       nrm_client_send_event(client, timestamp, sensor, scope, measurement);
       sleep(1);
     } while (measurement != 0);

     printf("done!");

     nrm_scope_destroy(scope);
	   nrm_sensor_destroy(&sensor);
	   nrm_client_destroy(&client);
	   nrm_finalize();

     exit(EXIT_SUCCESS);
   }

6. Logging Introduction
-----------------------

Initialize the NRM logging interface after ``nrm_init()``::

  ...
  nrm_init(NULL, NULL);
  nrm_log_init(stderr, "example");
  ...

Set a log level out of ``NRM_LOG_QUIET``, ``NRM_LOG_ERROR``, ``NRM_LOG_WARNING``, ``NRM_LOG_NORMAL``, ``NRM_LOG_INFO``, or ``NRM_LOG_DEBUG``::

  nrm_log_setlevel(NRM_LOG_DEBUG);

The ``nrm_log_error()`` ``nrm_log_warning()`` ``nrm_log_normal()`` ``nrm_log_info()`` and ``nrm_log_debug()`` logging functions
are available for logging messages up to the specified log level, with labels and line numbers also displayed::

  example:	debug:	example.c:	153:	Program initialized.
  example:	debug:	example.c:	171:	starting to detect measurements
  example:	debug:	example.c:	185:	sending measurements to NRM
  ...
  example:	debug:	src/client.c:	392:	crafting message
  ...
  example:	debug:	src/messages.c:	761:	received SEND:1
  example:	debug:	src/roles/client.c:	72:	client sending message
  {"type": "EVENT", "data": {"uuid": "...", "time": 1660321973749125893, "scope": {"uuid": "", "cpu": [32, ... 63], "numa": [], "gpu": []}, "value": 16.392918834073949}}


See the :ref:`C API logging section<logs>` for more information.