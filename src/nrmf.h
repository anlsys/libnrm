!*******************************************************************************
! * Copyright 2019 UChicago Argonne, LLC.
! * (c.f. AUTHORS, LICENSE)
! *
! * This file is part of the libnrm project.
! * For more info, see https://xgitlab.cels.anl.gov/argo/libnrm
! *
! * SPDX-License-Identifier: BSD-3-Clause
!*******************************************************************************

      integer                   NRM_PTR
      parameter(NRM_PTR=8)
      integer(kind=NRM_PTR)     nrmf_ctxt_create
      external                  nrmf_ctxt_create
      integer                   nrmf_ctxt_delete
      external                  nrmf_ctxt_delete
      integer(kind=NRM_PTR)     nrmf_init
      external                  nrmf_init
      integer                   nrmf_fini
      external                  nrmf_fini
      integer                   nrmf_send_progress
      external                  nrmf_send_progress
      integer                   nrmf_send_phase_context
      external                  nrmf_send_phase_context
