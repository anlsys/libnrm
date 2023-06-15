-- ******************************************************************************
--  Copyright 2021 UChicago Argonne, LLC.
--  (c.f. AUTHORS, LICENSE)
--
--  SPDX-License-Identifier: BSD-3-Clause
-- ******************************************************************************
--
-- types used by nrmd's configuration.
--
--
let types =
    -- This import defines a few base types that are common to manifest and
    -- configuration formats.
      ../common_types.dhall

let Verbosity =
    -- Daemon verbosity:
    --   Error: Only report errors
    --   Info: Be verbose
    --   Debug: Report all that can possibly be reported
      < Error | Info | Debug >

let SensorBehavior =
    -- Sensor Behavior.
    -- IntervalBased: a sensor that measures an average value for the time
    -- period since its last reading.
    -- Cumulative: a sensor that measures a counter. NRM must substract between
    -- subsequent counter values to obtain a reading for the corresponding time
    -- period.
    -- CumulativeWithCapacity: same as `Cumulative`, except the sensor gives
    -- bounded values that go back to zero after a given threshold. Typical of
    -- RAPL sysfs sensors, for instance .
      < IntervalBased | Cumulative | CumulativeWithCapacity : Double >

let Range =
    -- An inclusive range of values
      { lower : Double, upper : Double }

let DownstreamCfg =
    -- Configuration for the dowstream API
      { downstreamBindAddress : Text }

let UpstreamCfg =
    -- Configuration for the upstream API
      { upstreamBindAddress : Text, pubPort : Integer, rpcPort : Integer }

let Tag =
    -- Available tags for describing sensors. Tags are used for setting
    -- objective functions up.
      < TagPower
      | TagRapl
      | TagDownstreamThreadSignal
      | TagDownstreamCmdSignal
      | TagMinimize
      | TagMaximize
      >

let Sensor =
    -- Configuration for an arbitrary sensor.
      { sensorBinary : Text
      , sensorArguments : List Text
      , range : Range
      , tags : List Tag
      , sensorBehavior : SensorBehavior
      }

let Actuator =
    -- Configuration for an arbitrary actuator.
      { actuatorBinary : Text
      , actuatorArguments : List Text
      , actions : List Double
      , referenceAction : Double
      }

let ActuatorKV =
    -- Key-value representation for an actuator.
      { actuatorID : Text, actuator : Actuator }

let SensorKV =
    -- Key-value representation for a sensor.
      { sensorID : Text, sensor : Sensor }

let RaplCfg =
    -- Configuration for auto-discovered RAPL power sensors/actuators
      { raplPath : Text
      , raplActions : List types.Power
      , referencePower : types.Power
      }

let ActuatorValue =
    -- Actuator value configuration for the internal control loop configuration.
      { actuatorValueID : Text, actuatorValue : Double }

let Hint =
    -- Action space configuration for internal control loop.
      < Full
      | Only :
          { neHead : List ActuatorValue, neTail : List (List ActuatorValue) }
      >

let LearnCfg =
    -- Internal control loop algorithm type and hyperparameters.
      < Lagrange : { lagrange : Double }
      | Random : { seed : Integer }
      | Contextual : { horizon : Integer }
      >

let ControlCfg =
    -- Root control configuration.
    -- ControlCfg: configures an internal control loop
    -- ControlOff: bypass mode
      < ControlCfg :
          { minimumControlInterval : types.Time
          , minimumWaitInterval : types.Time
          , staticPower : types.Power
          , learnCfg : LearnCfg
          , speedThreshold : Double
          , referenceMeasurementRoundInterval : Integer
          , hint : Hint
          }
      | ControlOff
      >

let Cfg =
    -- The configuration type for `nrmd`.
    -- verbose : a verbosity level configuration.
    -- logfile : the main log file for the daemon.
    -- perfPath : the path/binary name of the linux perf executable.
    -- perfwrapperPath : the path/binary name of the nrm-perfwrapper executable
    -- libnrmPath : the path to the libnrm.so to use. leave at None to disable
    --              the feature globally (for all apps).
    -- downstreamCfg : Downstream API configuration.
    -- upstreamCfg : Upstream API configuration.
    -- raplCfg : RAPL configuration. Leave at None to disable the feature
    --           globally.
    -- controlCfg : resource control configuration
    -- passiveSensorFrequency : unified frequency for all NRM's passive sensors.
    -- extraStaticPassiveSensors : list of extra static passive sensors.
    -- extraStaticActuators : list of extra actuators.
      { verbose : Verbosity
      , logfile : Text
      , perfPath : Text
      , perfwrapperPath : Text
      , libnrmPath : Optional Text
      , downstreamCfg : DownstreamCfg
      , upstreamCfg : UpstreamCfg
      , raplCfg : Optional RaplCfg
      , controlCfg : ControlCfg
      , passiveSensorFrequency : types.Frequency
      , extraStaticPassiveSensors : List SensorKV
      , extraStaticActuators : List ActuatorKV
      }

let output =
      { Verbosity = Verbosity
      , UpstreamCfg = UpstreamCfg
      , DownstreamCfg = DownstreamCfg
      , Tag = Tag
      , SensorBehavior = SensorBehavior
      , Actuator = Actuator
      , SensorKV = SensorKV
      , Sensor = Sensor
      , ActuatorKV = ActuatorKV
      , Range = Range
      , PassiveSensorCfg = Sensor
      , ActuatorValue = ActuatorValue
      , ActuatorCfg = Actuator
      , LearnCfg = LearnCfg
      , RaplCfg = RaplCfg
      , ControlCfg = ControlCfg
      , Hint = Hint
      , Cfg = Cfg
      }

in  types ⫽ output
