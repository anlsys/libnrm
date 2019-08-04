!*******************************************************************************
! * Copyright 2019 UChicago Argonne, LLC.
! * (c.f. AUTHORS, LICENSE)
! *
! * This file is part of the libnrm project.
! * For more info, see https://xgitlab.cels.anl.gov/argo/libnrm
! *
! * SPDX-License-Identifier:
BSD-3-Clause
!*******************************************************************************

integer                   NRM_PTR
parameter(NRM_PTR=8)
integer(kind=NRM_PTR)     f_nrm_ctxt_create
external                  f_nrm_ctxt_create
integer                   f_nrm_ctxt_delete
external                  f_nrm_ctxt_delete
integer(kind=NRM_PTR)     f_nrm_init
external                  f_nrm_init
integer                   f_nrm_fini
external                  f_nrm_fini
integer                   f_nrm_send_progress
external                  f_nrm_send_progress
integer                   f_nrm_send_phase_context
external                  f_nrm_send_phase_context
