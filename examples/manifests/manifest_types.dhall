-- ******************************************************************************
--  Copyright 2021 UChicago Argonne, LLC.
--  (c.f. AUTHORS, LICENSE)
--
--  SPDX-License-Identifier: BSD-3-Clause
-- ******************************************************************************
let types =
    -- This import defines a few base types that are common to manifest and
    -- configuration formats.
      ../common_types.dhall

let Perfwrapper =
    -- Configuration for linux perf performance measurements. A fixed frequency
    -- is provided, along with a tentative upper bound on the maximum value of
    -- the sensor. This upper bound should be set to a low positive value if
    -- no a-priori information is known; NRM will then use a recursive-doubling
    -- strategy to maintain its own bound.
      { perfFreq : types.Frequency
      , perfLimit : Integer
      , perfEvent : Text
      }

let Instrumentation =
    -- Activate LD_PRELOAD based `libnrm` instrumentation. The only attribute that configures this feature
    -- is a message rate limitation.
      { ratelimit : types.Frequency }

let EnvVar =
    -- Key-value representation of environment variables
      { envName : Text, envValue : Text }

let AppActuator =
    -- Configuration for an arbitrary actuator.
      { actuatorBinary : Text
      , actuatorArguments : List Text
      , actuatorEnv : List EnvVar
      , actions : List Double
      , referenceAction : Double
      }

let AppActuatorKV =
    -- Key-value representation for an actuator.
      { actuatorID : Text, actuator : AppActuator }

let App =
    -- Application configuration. Two features can be enabled or disabled:
    -- perfwrapper: an optional linux perf configuration
    -- instrumentation: an optional libnrm instrumentation configuration
      { perfwrapper : Optional Perfwrapper
      , instrumentation : Optional Instrumentation
      , actuators : Optional (List AppActuatorKV)
      }

let Manifest =
    -- A manifest has a name, and an application configuration.
      { name : Text, app : App }

in    types
    ⫽ { Perfwrapper = Perfwrapper
      , Instrumentation = Instrumentation
      , App = App
      , Manifest = Manifest
      , AppActuator = AppActuator
      , AppActuatorKV = AppActuatorKV
      , EnvVar = EnvVar
      }
