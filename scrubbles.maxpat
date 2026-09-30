{
  "patcher": {
    "fileversion": 1,
    "appversion": {
      "major": 9,
      "minor": 1,
      "revision": 5,
      "architecture": "arm64",
      "modernui": 1
    },
    "classnamespace": "box",
    "rect": [
      100.0,
      100.0,
      840.0,
      1000.0
    ],
    "openinpresentation": 1,
    "default_fontsize": 12.0,
    "default_fontname": "Arial",
    "gridonopen": 0,
    "enablehscroll": 0,
    "enablevscroll": 0,
    "devicewidth": 760.0,
    "boxes": [
      {
        "box": {
          "id": "surface",
          "maxclass": "v8ui",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            0.0,
            0.0,
            760.0,
            169.0
          ],
          "outlettype": [
            ""
          ],
          "filename": "scrubbles-ui.js",
          "border": 0,
          "background": 1,
          "parameter_enable": 0,
          "presentation": 1,
          "presentation_rect": [
            0.0,
            0.0,
            760.0,
            169.0
          ],
          "varname": "surface",
          "textfile": {
            "filename": "scrubbles-ui.js",
            "flags": 0,
            "embed": 0,
            "autowatch": 1
          }
        }
      },
      {
        "box": {
          "id": "hold",
          "maxclass": "v8ui",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "filename": "scrubbles-hold.js",
          "border": 0,
          "varname": "p_hold",
          "patching_rect": [
            267.0,
            144.0,
            64.0,
            20.0
          ],
          "presentation": 1,
          "presentation_rect": [
            267.0,
            144.0,
            64.0,
            20.0
          ],
          "parameter_enable": 1,
          "parameter_mappable": 1,
          "annotation": "Freeze automatic travel while pressed. MIDI C0 (24) also freezes until released; C#0–E0 seek markers. Control notes are consumed. Run, Hold and playhead grab are independent.",
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Hold",
              "parameter_shortname": "Hold",
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_enum": [
                "Off",
                "Held"
              ],
              "parameter_initial": [
                0
              ],
              "parameter_initial_enable": 1,
              "parameter_invisible": 0,
              "parameter_modmode": 0,
              "parameter_order": 7,
              "parameter_defer": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "changed-hold",
          "maxclass": "newobj",
          "text": "prepend changed hold",
          "numinlets": 1,
          "numoutlets": 1,
          "outlettype": [
            ""
          ],
          "patching_rect": [
            460.0,
            470.0,
            130.0,
            22.0
          ]
        }
      },
      {
        "box": {
          "id": "input",
          "maxclass": "inlet",
          "numinlets": 0,
          "numoutlets": 1,
          "patching_rect": [
            20.0,
            220.0,
            30.0,
            30.0
          ],
          "outlettype": [
            ""
          ],
          "comment": "param / seekmarker / grab / map / unmap / bang",
          "index": 1
        }
      },
      {
        "box": {
          "id": "output",
          "maxclass": "outlet",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            700.0,
            930.0,
            30.0,
            30.0
          ],
          "comment": "state / telemetry / target / mapping / status",
          "index": 1
        }
      },
      {
        "box": {
          "id": "control",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 5,
          "patching_rect": [
            20.0,
            510.0,
            280.0,
            22.0
          ],
          "outlettype": [
            "",
            "",
            "",
            "",
            ""
          ],
          "text": "v8 scrubbles-control.js",
          "varname": "control",
          "saved_object_attributes": {
            "filename": "scrubbles-control.js"
          }
        }
      },
      {
        "box": {
          "id": "thisdevice",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 3,
          "patching_rect": [
            570.0,
            220.0,
            160.0,
            22.0
          ],
          "outlettype": [
            "bang",
            "int",
            "int"
          ],
          "text": "live.thisdevice"
        }
      },
      {
        "box": {
          "id": "initialize",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 2,
          "patching_rect": [
            570.0,
            255.0,
            60.0,
            22.0
          ],
          "outlettype": [
            "bang",
            "bang"
          ],
          "text": "t b b"
        }
      },
      {
        "box": {
          "id": "start-telemetry",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            640.0,
            290.0,
            30.0,
            22.0
          ],
          "outlettype": [
            ""
          ],
          "text": "1"
        }
      },
      {
        "box": {
          "id": "ready",
          "maxclass": "message",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            570.0,
            290.0,
            45.0,
            22.0
          ],
          "outlettype": [
            ""
          ],
          "text": "ready"
        }
      },
      {
        "box": {
          "id": "enabled",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            690.0,
            255.0,
            130.0,
            22.0
          ],
          "outlettype": [
            ""
          ],
          "text": "prepend enabled"
        }
      },
      {
        "box": {
          "id": "seed",
          "maxclass": "live.menu",
          "numinlets": 1,
          "numoutlets": 3,
          "patching_rect": [
            20.0,
            360.0,
            90.0,
            48.0
          ],
          "outlettype": [
            "",
            "",
            ""
          ],
          "hidden": 1,
          "parameter_enable": 1,
          "varname": "p_seed",
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Seed",
              "parameter_shortname": "Seed",
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 5,
              "parameter_initial": [
                5
              ],
              "parameter_initial_enable": 1,
              "parameter_invisible": 0,
              "parameter_modmode": 0,
              "parameter_unitstyle": 0,
              "parameter_order": 0,
              "parameter_defer": 1,
              "parameter_enum": [
                "Ramp",
                "Triangle",
                "Needle",
                "Staircase",
                "Gate",
                "Landscape"
              ]
            }
          }
        }
      },
      {
        "box": {
          "id": "changed-seed",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            20.0,
            435.0,
            105.0,
            22.0
          ],
          "outlettype": [
            ""
          ],
          "text": "prepend changed seed"
        }
      },
      {
        "box": {
          "id": "position",
          "maxclass": "live.dial",
          "numinlets": 1,
          "numoutlets": 2,
          "patching_rect": [
            130.0,
            360.0,
            90.0,
            48.0
          ],
          "outlettype": [
            "",
            ""
          ],
          "hidden": 1,
          "parameter_enable": 1,
          "varname": "p_position",
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Position",
              "parameter_shortname": "Position",
              "parameter_type": 0,
              "parameter_mmin": 0.0,
              "parameter_mmax": 1.0,
              "parameter_initial": [
                0.0
              ],
              "parameter_initial_enable": 1,
              "parameter_invisible": 0,
              "parameter_modmode": 0,
              "parameter_unitstyle": 1,
              "parameter_order": 1,
              "parameter_defer": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "changed-position",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            130.0,
            435.0,
            105.0,
            22.0
          ],
          "outlettype": [
            ""
          ],
          "text": "prepend changed position"
        }
      },
      {
        "box": {
          "id": "rate",
          "maxclass": "live.dial",
          "numinlets": 1,
          "numoutlets": 2,
          "patching_rect": [
            240.0,
            360.0,
            90.0,
            48.0
          ],
          "outlettype": [
            "",
            ""
          ],
          "hidden": 1,
          "parameter_enable": 1,
          "varname": "p_rate",
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Rate",
              "parameter_shortname": "Rate",
              "parameter_type": 0,
              "parameter_mmin": 0.02,
              "parameter_mmax": 8.0,
              "parameter_initial": [
                0.25
              ],
              "parameter_initial_enable": 1,
              "parameter_invisible": 0,
              "parameter_modmode": 0,
              "parameter_unitstyle": 3,
              "parameter_order": 2,
              "parameter_defer": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "changed-rate",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            240.0,
            435.0,
            105.0,
            22.0
          ],
          "outlettype": [
            ""
          ],
          "text": "prepend changed rate"
        }
      },
      {
        "box": {
          "id": "amount",
          "maxclass": "live.dial",
          "numinlets": 1,
          "numoutlets": 2,
          "patching_rect": [
            350.0,
            360.0,
            90.0,
            48.0
          ],
          "outlettype": [
            "",
            ""
          ],
          "hidden": 1,
          "parameter_enable": 1,
          "varname": "p_amount",
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Amount",
              "parameter_shortname": "Amount",
              "parameter_type": 0,
              "parameter_mmin": 0.0,
              "parameter_mmax": 1.0,
              "parameter_initial": [
                0.5
              ],
              "parameter_initial_enable": 1,
              "parameter_invisible": 0,
              "parameter_modmode": 0,
              "parameter_unitstyle": 1,
              "parameter_order": 3,
              "parameter_defer": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "changed-amount",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            350.0,
            435.0,
            105.0,
            22.0
          ],
          "outlettype": [
            ""
          ],
          "text": "prepend changed amount"
        }
      },
      {
        "box": {
          "id": "running",
          "maxclass": "live.toggle",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            460.0,
            360.0,
            90.0,
            48.0
          ],
          "outlettype": [
            ""
          ],
          "hidden": 1,
          "parameter_enable": 1,
          "varname": "p_running",
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Run",
              "parameter_shortname": "Run",
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                1
              ],
              "parameter_initial_enable": 1,
              "parameter_invisible": 0,
              "parameter_modmode": 0,
              "parameter_unitstyle": 0,
              "parameter_order": 4,
              "parameter_defer": 1,
              "parameter_enum": [
                "Stop",
                "Run"
              ]
            }
          }
        }
      },
      {
        "box": {
          "id": "changed-running",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            460.0,
            435.0,
            105.0,
            22.0
          ],
          "outlettype": [
            ""
          ],
          "text": "prepend changed running"
        }
      },
      {
        "box": {
          "id": "reverse",
          "maxclass": "live.toggle",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            570.0,
            360.0,
            90.0,
            48.0
          ],
          "outlettype": [
            ""
          ],
          "hidden": 1,
          "parameter_enable": 1,
          "varname": "p_reverse",
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Reverse",
              "parameter_shortname": "Reverse",
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                0
              ],
              "parameter_initial_enable": 1,
              "parameter_invisible": 0,
              "parameter_modmode": 0,
              "parameter_unitstyle": 0,
              "parameter_order": 5,
              "parameter_defer": 1,
              "parameter_enum": [
                "Forward",
                "Reverse"
              ]
            }
          }
        }
      },
      {
        "box": {
          "id": "changed-reverse",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            570.0,
            435.0,
            105.0,
            22.0
          ],
          "outlettype": [
            ""
          ],
          "text": "prepend changed reverse"
        }
      },
      {
        "box": {
          "id": "bipolar",
          "maxclass": "live.toggle",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            680.0,
            360.0,
            90.0,
            48.0
          ],
          "outlettype": [
            ""
          ],
          "hidden": 1,
          "parameter_enable": 1,
          "varname": "p_bipolar",
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Bipolar",
              "parameter_shortname": "Bipolar",
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                1
              ],
              "parameter_initial_enable": 1,
              "parameter_invisible": 0,
              "parameter_modmode": 0,
              "parameter_unitstyle": 0,
              "parameter_order": 6,
              "parameter_defer": 1,
              "parameter_enum": [
                "Unipolar",
                "Bipolar"
              ]
            }
          }
        }
      },
      {
        "box": {
          "id": "changed-bipolar",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            680.0,
            435.0,
            105.0,
            22.0
          ],
          "outlettype": [
            ""
          ],
          "text": "prepend changed bipolar"
        }
      },
      {
        "box": {
          "id": "engine",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 3,
          "patching_rect": [
            20.0,
            605.0,
            225.0,
            22.0
          ],
          "outlettype": [
            "signal",
            "signal",
            "signal"
          ],
          "text": "gen~ scrubbles-engine",
          "varname": "engine"
        }
      },
      {
        "box": {
          "id": "mapper",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 5,
          "patching_rect": [
            390.0,
            605.0,
            265.0,
            22.0
          ],
          "outlettype": [
            "",
            "",
            "",
            "int",
            ""
          ],
          "text": "live.map @strict 1",
          "varname": "mapping",
          "saved_object_attributes": {
            "_persistence": 1
          }
        }
      },
      {
        "box": {
          "id": "mapped",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            390.0,
            665.0,
            130.0,
            22.0
          ],
          "outlettype": [
            ""
          ],
          "text": "prepend mapped"
        }
      },
      {
        "box": {
          "id": "targetname",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            535.0,
            700.0,
            150.0,
            22.0
          ],
          "outlettype": [
            ""
          ],
          "text": "prepend targetname"
        }
      },
      {
        "box": {
          "id": "mappingstate",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            670.0,
            740.0,
            155.0,
            22.0
          ],
          "outlettype": [
            ""
          ],
          "text": "prepend mappingstate"
        }
      },
      {
        "box": {
          "id": "modulator",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            20.0,
            860.0,
            225.0,
            22.0
          ],
          "outlettype": [
            ""
          ],
          "text": "live.modulate~ @depth 0.5",
          "varname": "modulation"
        }
      },
      {
        "box": {
          "id": "telemetry-clock",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            270.0,
            605.0,
            150.0,
            22.0
          ],
          "outlettype": [
            ""
          ],
          "text": "qmetro 33.333333"
        }
      },
      {
        "box": {
          "id": "snapshot-order",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 2,
          "patching_rect": [
            270.0,
            645.0,
            50.0,
            22.0
          ],
          "outlettype": [
            "bang",
            "bang"
          ],
          "text": "t b b"
        }
      },
      {
        "box": {
          "id": "snapshot-position",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            160.0,
            700.0,
            85.0,
            22.0
          ],
          "outlettype": [
            "float"
          ],
          "text": "snapshot~"
        }
      },
      {
        "box": {
          "id": "snapshot-value",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            270.0,
            700.0,
            85.0,
            22.0
          ],
          "outlettype": [
            "float"
          ],
          "text": "snapshot~"
        }
      },
      {
        "box": {
          "id": "telemetry-values",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            210.0,
            755.0,
            100.0,
            22.0
          ],
          "outlettype": [
            ""
          ],
          "text": "pack 0. 0."
        }
      },
      {
        "box": {
          "id": "telemetry",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            210.0,
            795.0,
            145.0,
            22.0
          ],
          "outlettype": [
            ""
          ],
          "text": "prepend telemetry"
        }
      },
      {
        "box": {
          "id": "note-parameters",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            20.0,
            315.0,
            500.0,
            22.0
          ],
          "text": "Hidden Live parameters own defaults, persistence, MIDI mapping and automation."
        }
      },
      {
        "box": {
          "id": "note-engine",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            20.0,
            565.0,
            460.0,
            22.0
          ],
          "text": "gen~: modulation signal / actual position / raw contour. No UI-rate DSP."
        }
      },
      {
        "box": {
          "id": "note-mapping",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            385.0,
            565.0,
            420.0,
            22.0
          ],
          "text": "Native persistent mapping; IDs only. Never writes target base values."
        }
      },
      {
        "box": {
          "id": "note-snapshot",
          "maxclass": "comment",
          "numinlets": 1,
          "numoutlets": 0,
          "patching_rect": [
            390.0,
            820.0,
            405.0,
            36.0
          ],
          "text": "30 Hz snapshots report DSP state. The transport does not depend on this clock.",
          "linecount": 2
        }
      },
      {
        "box": {
          "id": "midi-input",
          "maxclass": "inlet",
          "numinlets": 0,
          "numoutlets": 1,
          "patching_rect": [
            90,
            220,
            30,
            30
          ],
          "index": 2,
          "outlettype": [
            ""
          ],
          "comment": "Raw MIDI for controls; parent consumes C0–E0 before MIDI output"
        }
      },
      {
        "box": {
          "id": "midi-parse",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 8,
          "patching_rect": [
            870,
            245,
            420,
            22
          ],
          "text": "midiparse",
          "outlettype": [
            "",
            "",
            "",
            "int",
            "int",
            "",
            "int",
            ""
          ]
        }
      },
      {
        "box": {
          "id": "note-channel",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            870,
            290,
            100,
            22
          ],
          "text": "join 2 @triggers 0",
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "cc-channel",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            1025,
            290,
            100,
            22
          ],
          "text": "join 2 @triggers 0",
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "note-event",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            870,
            325,
            105,
            22
          ],
          "text": "prepend note",
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "cc-event",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            1025,
            325,
            100,
            22
          ],
          "text": "prepend cc",
          "outlettype": [
            ""
          ]
        }
      },
      {
        "box": {
          "id": "marker1",
          "maxclass": "live.dial",
          "numinlets": 1,
          "numoutlets": 2,
          "patching_rect": [870, 510, 90, 48],
          "outlettype": ["", ""],
          "hidden": 1,
          "parameter_enable": 1,
          "varname": "p_marker1",
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Marker 1",
              "parameter_shortname": "C#0",
              "parameter_type": 0,
              "parameter_mmin": 0.0,
              "parameter_mmax": 1.0,
              "parameter_initial": [0.0],
              "parameter_initial_enable": 1,
              "parameter_invisible": 0,
              "parameter_modmode": 0,
              "parameter_unitstyle": 1,
              "parameter_order": 11,
              "parameter_defer": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "marker2",
          "maxclass": "live.dial",
          "numinlets": 1,
          "numoutlets": 2,
          "patching_rect": [990, 510, 90, 48],
          "outlettype": ["", ""],
          "hidden": 1,
          "parameter_enable": 1,
          "varname": "p_marker2",
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Marker 2",
              "parameter_shortname": "D0",
              "parameter_type": 0,
              "parameter_mmin": 0.0,
              "parameter_mmax": 1.0,
              "parameter_initial": [0.25],
              "parameter_initial_enable": 1,
              "parameter_invisible": 0,
              "parameter_modmode": 0,
              "parameter_unitstyle": 1,
              "parameter_order": 12,
              "parameter_defer": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "marker3",
          "maxclass": "live.dial",
          "numinlets": 1,
          "numoutlets": 2,
          "patching_rect": [1110, 510, 90, 48],
          "outlettype": ["", ""],
          "hidden": 1,
          "parameter_enable": 1,
          "varname": "p_marker3",
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Marker 3",
              "parameter_shortname": "D#0",
              "parameter_type": 0,
              "parameter_mmin": 0.0,
              "parameter_mmax": 1.0,
              "parameter_initial": [0.5],
              "parameter_initial_enable": 1,
              "parameter_invisible": 0,
              "parameter_modmode": 0,
              "parameter_unitstyle": 1,
              "parameter_order": 13,
              "parameter_defer": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "marker4",
          "maxclass": "live.dial",
          "numinlets": 1,
          "numoutlets": 2,
          "patching_rect": [1230, 510, 90, 48],
          "outlettype": ["", ""],
          "hidden": 1,
          "parameter_enable": 1,
          "varname": "p_marker4",
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Marker 4",
              "parameter_shortname": "E0",
              "parameter_type": 0,
              "parameter_mmin": 0.0,
              "parameter_mmax": 1.0,
              "parameter_initial": [0.75],
              "parameter_initial_enable": 1,
              "parameter_invisible": 0,
              "parameter_modmode": 0,
              "parameter_unitstyle": 1,
              "parameter_order": 14,
              "parameter_defer": 1
            }
          }
        }
      },
      {
        "box": {
          "id": "sync",
          "maxclass": "live.toggle",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            20.0,
            480.0,
            90.0,
            48.0
          ],
          "outlettype": [
            ""
          ],
          "hidden": 1,
          "parameter_enable": 1,
          "varname": "p_sync",
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Sync",
              "parameter_shortname": "Sync",
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 1,
              "parameter_initial": [
                1
              ],
              "parameter_initial_enable": 1,
              "parameter_invisible": 0,
              "parameter_modmode": 0,
              "parameter_unitstyle": 0,
              "parameter_order": 9,
              "parameter_defer": 1,
              "parameter_enum": [
                "Free",
                "Sync"
              ]
            }
          }
        }
      },
      {
        "box": {
          "id": "changed-sync",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            20.0,
            545.0,
            130.0,
            22.0
          ],
          "outlettype": [
            ""
          ],
          "text": "prepend changed sync"
        }
      },
      {
        "box": {
          "id": "division",
          "maxclass": "live.menu",
          "numinlets": 1,
          "numoutlets": 3,
          "patching_rect": [
            175.0,
            480.0,
            90.0,
            48.0
          ],
          "outlettype": [
            "",
            "",
            ""
          ],
          "hidden": 1,
          "parameter_enable": 1,
          "varname": "p_division",
          "saved_attribute_attributes": {
            "valueof": {
              "parameter_longname": "Division",
              "parameter_shortname": "Division",
              "parameter_type": 2,
              "parameter_mmin": 0,
              "parameter_mmax": 8,
              "parameter_initial": [
                6
              ],
              "parameter_initial_enable": 1,
              "parameter_invisible": 0,
              "parameter_modmode": 0,
              "parameter_unitstyle": 0,
              "parameter_order": 10,
              "parameter_defer": 1,
              "parameter_enum": [
                "1/64",
                "1/32",
                "1/16",
                "1/8",
                "1/4",
                "1/2",
                "1 bar",
                "2 bars",
                "4 bars"
              ]
            }
          }
        }
      },
      {
        "box": {
          "id": "changed-division",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            175.0,
            545.0,
            150.0,
            22.0
          ],
          "outlettype": [
            ""
          ],
          "text": "prepend changed division"
        }
      },
      {
        "box": {
          "id": "syncclock",
          "maxclass": "newobj",
          "numinlets": 2,
          "numoutlets": 1,
          "patching_rect": [
            340.0,
            545.0,
            160.0,
            22.0
          ],
          "outlettype": [
            "signal"
          ],
          "text": "phasor~ 1.0.0 @lock 1",
          "varname": "syncclock"
        }
      },
      {
        "box": {
          "id": "host-sync",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 9,
          "patching_rect": [
            850.0,
            480.0,
            160.0,
            22.0
          ],
          "outlettype": [
            "int",
            "int",
            "int",
            "float",
            "list",
            "float",
            "float",
            "int",
            "int"
          ],
          "text": "plugsync~"
        }
      },
      {
        "box": {
          "id": "host-playing-change",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 3,
          "patching_rect": [
            850.0,
            515.0,
            60.0,
            22.0
          ],
          "outlettype": [
            "",
            "int",
            "int"
          ],
          "text": "change"
        }
      },
      {
        "box": {
          "id": "host-playing",
          "maxclass": "newobj",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [
            850.0,
            550.0,
            140.0,
            22.0
          ],
          "outlettype": [
            ""
          ],
          "text": "prepend hostplaying"
        }
      },
      {
        "box": {
          "id": "changed-marker1",
          "maxclass": "newobj",
          "text": "prepend changed marker1",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [870, 580, 170, 22]
        }
      },
      {
        "box": {
          "id": "changed-marker2",
          "maxclass": "newobj",
          "text": "prepend changed marker2",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [990, 610, 170, 22]
        }
      },
      {
        "box": {
          "id": "changed-marker3",
          "maxclass": "newobj",
          "text": "prepend changed marker3",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [1110, 640, 170, 22]
        }
      },
      {
        "box": {
          "id": "changed-marker4",
          "maxclass": "newobj",
          "text": "prepend changed marker4",
          "numinlets": 1,
          "numoutlets": 1,
          "patching_rect": [1230, 670, 170, 22]
        }
      }
    ],
    "lines": [
      {
        "patchline": {
          "source": [
            "sync",
            0
          ],
          "destination": [
            "changed-sync",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "changed-sync",
            0
          ],
          "destination": [
            "control",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "division",
            0
          ],
          "destination": [
            "changed-division",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "changed-division",
            0
          ],
          "destination": [
            "control",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "syncclock",
            0
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "host-sync",
            0
          ],
          "destination": [
            "host-playing-change",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "host-playing-change",
            0
          ],
          "destination": [
            "host-playing",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "host-playing",
            0
          ],
          "destination": [
            "control",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "hold",
            0
          ],
          "destination": [
            "changed-hold",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "changed-hold",
            0
          ],
          "destination": [
            "control",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "seed",
            0
          ],
          "destination": [
            "changed-seed",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "changed-seed",
            0
          ],
          "destination": [
            "control",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "position",
            0
          ],
          "destination": [
            "changed-position",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "changed-position",
            0
          ],
          "destination": [
            "control",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "rate",
            0
          ],
          "destination": [
            "changed-rate",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "changed-rate",
            0
          ],
          "destination": [
            "control",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "amount",
            0
          ],
          "destination": [
            "changed-amount",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "changed-amount",
            0
          ],
          "destination": [
            "control",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "running",
            0
          ],
          "destination": [
            "changed-running",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "changed-running",
            0
          ],
          "destination": [
            "control",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "reverse",
            0
          ],
          "destination": [
            "changed-reverse",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "changed-reverse",
            0
          ],
          "destination": [
            "control",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "bipolar",
            0
          ],
          "destination": [
            "changed-bipolar",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "changed-bipolar",
            0
          ],
          "destination": [
            "control",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "input",
            0
          ],
          "destination": [
            "control",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "surface",
            0
          ],
          "destination": [
            "control",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "control",
            0
          ],
          "destination": [
            "surface",
            0
          ],
          "order": 0
        }
      },
      {
        "patchline": {
          "source": [
            "control",
            0
          ],
          "destination": [
            "output",
            0
          ],
          "order": 1
        }
      },
      {
        "patchline": {
          "source": [
            "control",
            1
          ],
          "destination": [
            "engine",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "control",
            2
          ],
          "destination": [
            "modulator",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "control",
            3
          ],
          "destination": [
            "mapper",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "control",
            4
          ],
          "destination": [
            "modulator",
            1
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "thisdevice",
            0
          ],
          "destination": [
            "initialize",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "thisdevice",
            1
          ],
          "destination": [
            "enabled",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "enabled",
            0
          ],
          "destination": [
            "control",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "initialize",
            1
          ],
          "destination": [
            "start-telemetry",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "start-telemetry",
            0
          ],
          "destination": [
            "telemetry-clock",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "initialize",
            0
          ],
          "destination": [
            "ready",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "ready",
            0
          ],
          "destination": [
            "control",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "mapper",
            1
          ],
          "destination": [
            "mapped",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "mapped",
            0
          ],
          "destination": [
            "control",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "mapper",
            2
          ],
          "destination": [
            "targetname",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "targetname",
            0
          ],
          "destination": [
            "control",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "mapper",
            3
          ],
          "destination": [
            "mappingstate",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "mappingstate",
            0
          ],
          "destination": [
            "control",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "engine",
            0
          ],
          "destination": [
            "modulator",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "engine",
            1
          ],
          "destination": [
            "snapshot-position",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "engine",
            2
          ],
          "destination": [
            "snapshot-value",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "telemetry-clock",
            0
          ],
          "destination": [
            "snapshot-order",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "snapshot-order",
            1
          ],
          "destination": [
            "snapshot-value",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "snapshot-order",
            0
          ],
          "destination": [
            "snapshot-position",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "snapshot-value",
            0
          ],
          "destination": [
            "telemetry-values",
            1
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "snapshot-position",
            0
          ],
          "destination": [
            "telemetry-values",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "telemetry-values",
            0
          ],
          "destination": [
            "telemetry",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "telemetry",
            0
          ],
          "destination": [
            "surface",
            0
          ],
          "order": 0
        }
      },
      {
        "patchline": {
          "source": [
            "telemetry",
            0
          ],
          "destination": [
            "output",
            0
          ],
          "order": 1
        }
      },
      {
        "patchline": {
          "source": [
            "midi-input",
            0
          ],
          "destination": [
            "midi-parse",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "midi-parse",
            0
          ],
          "destination": [
            "note-channel",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "midi-parse",
            2
          ],
          "destination": [
            "cc-channel",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "midi-parse",
            6
          ],
          "destination": [
            "note-channel",
            1
          ],
          "order": 0
        }
      },
      {
        "patchline": {
          "source": [
            "midi-parse",
            6
          ],
          "destination": [
            "cc-channel",
            1
          ],
          "order": 1
        }
      },
      {
        "patchline": {
          "source": [
            "note-channel",
            0
          ],
          "destination": [
            "note-event",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "cc-channel",
            0
          ],
          "destination": [
            "cc-event",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "note-event",
            0
          ],
          "destination": [
            "control",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": [
            "cc-event",
            0
          ],
          "destination": [
            "control",
            0
          ]
        }
      },
      {
        "patchline": {
          "source": ["marker1", 0],
          "destination": ["changed-marker1", 0]
        }
      },
      {
        "patchline": {
          "source": ["changed-marker1", 0],
          "destination": ["control", 0]
        }
      },
      {
        "patchline": {
          "source": ["marker2", 0],
          "destination": ["changed-marker2", 0]
        }
      },
      {
        "patchline": {
          "source": ["changed-marker2", 0],
          "destination": ["control", 0]
        }
      },
      {
        "patchline": {
          "source": ["marker3", 0],
          "destination": ["changed-marker3", 0]
        }
      },
      {
        "patchline": {
          "source": ["changed-marker3", 0],
          "destination": ["control", 0]
        }
      },
      {
        "patchline": {
          "source": ["marker4", 0],
          "destination": ["changed-marker4", 0]
        }
      },
      {
        "patchline": {
          "source": ["changed-marker4", 0],
          "destination": ["control", 0]
        }
      }
    ],
    "parameters": {
      "seed": [
        "Seed",
        "Seed",
        0
      ],
      "position": [
        "Position",
        "Position",
        0
      ],
      "rate": [
        "Rate",
        "Rate",
        0
      ],
      "amount": [
        "Amount",
        "Amount",
        0
      ],
      "running": [
        "Run",
        "Run",
        0
      ],
      "reverse": [
        "Reverse",
        "Reverse",
        0
      ],
      "bipolar": [
        "Bipolar",
        "Bipolar",
        0
      ],
      "hold": [
        "Hold",
        "Hold",
        0
      ],
      "sync": [
        "Sync",
        "Sync",
        0
      ],
      "division": [
        "Division",
        "Division",
        0
      ],
      "parameterbanks": {
        "0": {
          "index": 0,
          "name": "Scrubbles",
          "parameters": [
            "seed",
            "position",
            "rate",
            "amount",
            "running",
            "reverse",
            "bipolar",
            "hold"
          ]
        },
        "1": {
          "index": 1,
          "name": "Timing / Markers",
          "parameters": [
            "sync",
            "division",
            "marker1",
            "marker2",
            "marker3",
            "marker4",
            "-",
            "-"
          ]
        }
      },
      "inherited_shortname": 1,
      "marker1": ["Marker 1", "C#0", 0],
      "marker2": ["Marker 2", "D0", 0],
      "marker3": ["Marker 3", "D#0", 0],
      "marker4": ["Marker 4", "E0", 0]
    },
    "dependency_cache": [
      {
        "name": "scrubbles-hold.js",
        "bootpath": ".",
        "patcherrelativepath": ".",
        "type": "TEXT",
        "implicit": 1
      },
      {
        "name": "scrubbles-ui.js",
        "bootpath": ".",
        "patcherrelativepath": ".",
        "type": "TEXT",
        "implicit": 1
      },
      {
        "name": "scrubbles-control.js",
        "bootpath": ".",
        "patcherrelativepath": ".",
        "type": "TEXT",
        "implicit": 1
      },
      {
        "name": "scrubbles-engine.gendsp",
        "bootpath": ".",
        "patcherrelativepath": ".",
        "type": "gDSP",
        "implicit": 1
      }
    ],
    "autosave": 0
  }
}
