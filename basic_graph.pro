QT += widgets
QMAKE_CXXFLAGS += -std=c++17 -mfpmath=sse -fstack-protector-all -W -Wall -Wextra -Wunused -Wcast-align -Werror -pedantic \
-pedantic-errors -Wfloat-equal -Wpointer-arith -Wformat-security -Wmissing-format-attribute -Wformat=1 -Wwrite-strings \
-Wcast-align -Wno-long-long -Woverloaded-virtual -Wnon-virtual-dtor -Wcast-qual -Wno-suggest-attribute=format

HEADERS       = window.h \
  calculator.h \
  functions_and_derivatives.h
SOURCES       = main.cpp \
                calculator.cpp \
                functions_and_derivatives.cpp \
                window.cpp
