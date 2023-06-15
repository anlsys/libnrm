let types = ./types_manifest.dhall

let default = ./defaults_manifest.dhall

in      default
      ⫽ { name = "linuxperf-wrapped"
        , app =
              default.app
            ⫽ { perfwrapper = Some { perfFreq.hertz = 1.0,
	    			     perfLimit = +100000,
				     perfEvent = "instructions" }
              }
        }
    : types.Manifest
