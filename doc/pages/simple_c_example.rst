Using libnrm in your C applications
===================================

1. Include NRM and create a context
-----------------------------------

To make code report progress to NRM, we need to:

- Include the library in the same way you include your other dependencies::

    # include <nrm.h>

- Create a ``nrm_context`` structure, corresponding to this program::

    struct nrm_context *context;
    context = nrm_ctxt_create();

- Initialize this NRM context::

    nrm_init(context, "my-program", 0, 0);  // zeros refer to rank, thread

2. Determine measurements and create scopes
-------------------------------------------

- Create a ``nrm_scope`` structure **for each** measurement to report. If only reporting
  a single measurement, the following is all you need::

    struct nrm_scope *scope;
    scope = nrm_scope_create();

  However, if reporting multiple measurements, feel free to use the ``nrm_scope_t`` type
  to create an array of scopes, e.g.::

    nrm_scope_t *nrm_scopes[NUM_MEASUREMENTS];
    for (int i=0; i<NUM_MEASUREMENTS, i++){
      scope = nrm_scope_create();
      nrm_scopes[i] = scope;
    }

- Configure your scope(s) to contain a resource type and corresponding logical indexes for that resource::

    // For example, for each logical CPU (calculated separately)
    for (int i=0; i<num_logical_cpus; i++) {
      nrm_scope_add(scope, NRM_SCOPE_TYPE_CPU, i);
    }

Other scope types include ``NRM_SCOPE_TYPE_NUMA`` and ``NRM_SCOPE_TYPE_GPU``.

3. Send measurements and close down
-----------------------------------

- Report measurement progress to NRM using the context and a scope::

    int measurement = example_get_cpu_measurement();
    nrm_send_progress(context, measurement, scope);

- Finalize the connection, delete the NRM scope(s) and context::

    nrm_fini(context);
    nrm_scope_delete(scope);
    nrm_ctxt_delete(context);

4. Complete example
-------------------

The best way to understand how to make your application report progress to NRM is to use an example.
The following simple program uses each of the previously described NRM components
to periodically report a measurement that corresponds to a set of logical CPUs on some system::

   # include <stdio.h>
   # include <unistd.h>
   # include <nrm.h>

   int main()
   {
     int i, num_logical_cpus, measurement;
     struct nrm_context *context;
     struct nrm_scope *scope;

     context = nrm_ctxt_create();
     nrm_init(context, "my-program", 0, 0);

     scope = nrm_scope_create();

     num_logical_cpus = example_get_num_logical_cpus();
     for (int i=0; i<num_logical_cpus; i++) {
       nrm_scope_add(scope, NRM_SCOPE_TYPE_CPU, i);
     }

     printf("hello\n")

     do {
       measurement = example_get_cpu_measurement();
       nrm_send_progress(context, measurement, scope);
       sleep(1);
     } while (measurement != 0);

     printf("done!");

     nrm_fini(context);
     nrm_scope_delete(scope);

     return 0;
   }
