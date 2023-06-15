-- ******************************************************************************
--  Copyright 2021 UChicago Argonne, LLC.
--  (c.f. AUTHORS, LICENSE)
--
--  SPDX-License-Identifier: BSD-3-Clause
-- ******************************************************************************

let t = ./types_manifest.dhall

in  { name = "default"
    , app =
        { perfwrapper = None t.Perfwrapper
        , instrumentation = None t.Instrumentation
        , actuators = None (List t.AppActuatorKV)
        }
    }
