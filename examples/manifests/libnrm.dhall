let types = ./types_manifest.dhall

let default = ./defaults_manifest.dhall

in      default
      ⫽ { name = "libnrm-wrapped"
        , app =
              default.app
            ⫽ { instrumentation = Some { ratelimit.hertz = 1000000.0 } }
        }
    : types.Manifest
