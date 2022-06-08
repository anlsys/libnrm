Using libnrm in your Fortran application
========================================
.. highlight:: Fortran

The same thing can be done with a Fortran application, using the Fortran
interface of this library. Let's take a similar example::

   implicit none

   integer i

   print*, "hello"

   do i=1, 4
     print*, "number", i
   end do

   print*, "done!"

The functions in the Fortran interface are similar to the ones from the C API,
only with a ``f_`` in front.
To talk to NRM, the code becomes::

   implicit none

   include 'f_nrm.h'
   include(kind=NRM_PTR) context

   integer rc, i

   print*, "hello"
   rc = f_nrm_ctxt_create(context);
   rc = f_nrm_init(context, 'example', len('example'))

   do i=1, 4
     print*, "number", i
     progress = 1.0
     rc = f_nrm_send_progress(context, progress)
   end do

   print*, "done!"
   rc = f_nrm_fini(context)
   rc = f_nrm_ctxt_delete(context)
