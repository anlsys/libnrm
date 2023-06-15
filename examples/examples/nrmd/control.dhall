let types = ./types_nrmd.dhall

let default = ./defaults_nrmd.dhall

let action1 = { microwatts = 1.0e8 }

let action2 = { microwatts = 2.0e8 }

in      default
      ⫽ { raplCfg = Some
            { raplPath = "/sys/devices/virtual/powercap/intel-rapl"
            , raplActions = [ action1, action2 ]
            , referencePower.microwatts = 2.5e8
            }
        , controlCfg =
            types.ControlCfg.ControlCfg
              { minimumControlInterval.microseconds = 100000.0
              , minimumWaitInterval.microseconds = 100000.0
              , staticPower.microwatts = 2.0e8
              , learnCfg = types.LearnCfg.Contextual { horizon = +4000 }
              , speedThreshold = 1.1
              , referenceMeasurementRoundInterval = +6
              , hint = types.Hint.Full
              }
        , passiveSensorFrequency.hertz = 1.0
        }
    : types.Cfg
