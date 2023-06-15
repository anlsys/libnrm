let types = ./types_nrmd.dhall

let default = ./defaults_nrmd.dhall

in      default
      ⫽ { extraStaticPassiveSensors =
          [ { sensorID = "example extra static passive power sensor"
            , sensor =
                { sensorBinary = "echo"
                , sensorArguments = [ "30" ]
                , range = { lower = 1.0, upper = 40.0 }
                , tags = [ types.Tag.TagPower ]
                , sensorBehavior = types.SensorBehavior.IntervalBased
                }
            }
          ]
        }
    : types.Cfg
