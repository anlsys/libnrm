let t = ./types_nrmd.dhall

in    { verbose = t.Verbosity.Error
      , logfile = "/tmp/nrm.log"
      , perfPath = "perf"
      , perfwrapperPath = "nrm-perfwrapper"
      , libnrmPath = None Text
      , downstreamCfg.downstreamBindAddress = "ipc:///tmp/nrm-downstream-event"
      , upstreamCfg =
          { upstreamBindAddress = "*", pubPort = +2345, rpcPort = +3456 }
      , raplCfg = Some
          { raplPath = "/sys/devices/virtual/powercap/intel-rapl"
          , raplActions = [ { microwatts = 1.0e8 }, { microwatts = 2.0e8 } ]
          , referencePower.microwatts = 2.5e8
          }
      , controlCfg = t.ControlCfg.ControlOff
      , passiveSensorFrequency.hertz = 1.0
      , extraStaticPassiveSensors = [] : List t.SensorKV
      , extraStaticActuators = [] : List t.ActuatorKV
      }
    : t.Cfg
