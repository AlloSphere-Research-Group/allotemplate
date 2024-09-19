if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(FATAL_ERROR
    "\nIn-source builds are not allowed.\n"
    "Remove CMakeFiles/, CMakeCache.txt and retry with a path to build tree:\n"
    " e.g.) cmake -B <destination>\n"
  )
endif()