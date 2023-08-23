{
  "targets": [
    {
      "target_name": "printer",
      "sources": [
        "main.cpp",
        "./lib/base64/base64.cpp",
        "./lib/win-iconv/win_iconv.c"
      ],
      "include_dirs": [],
      "libraries": [],
      "win_delay_load_hook": "true"
    }
  ]
}