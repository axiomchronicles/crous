{
  "targets": [
    {
      "target_name": "crous",
      "sources": [
        "src/crous_node.c",
        "crous_core/src/core/errors.c",
        "crous_core/src/core/arena.c",
        "crous_core/src/core/value.c",
        "crous_core/src/core/version.c",
        "crous_core/src/utils/token.c",
        "crous_core/src/lexer/lexer.c",
        "crous_core/src/parser/parser.c",
        "crous_core/src/binary/binary.c",
        "crous_core/src/flux/flux_lexer.c",
        "crous_core/src/flux/flux_parser.c",
        "crous_core/src/flux/flux_serializer.c"
      ],
      "include_dirs": [
        "crous_core/include"
      ],
      "cflags": [
        "-O3",
        "-Wall",
        "-Wextra",
        "-std=c99"
      ],
      "cflags_cc": [
        "-O3",
        "-Wall",
        "-Wextra"
      ],
      "xcode_settings": {
        "GCC_OPTIMIZATION_LEVEL": "3",
        "WARNING_CFLAGS": [
          "-Wall",
          "-Wextra"
        ],
        "OTHER_CFLAGS": [
          "-std=c99"
        ]
      },
      "msvs_settings": {
        "VCCLCompilerTool": {
          "Optimization": 3
        }
      }
    }
  ]
}
