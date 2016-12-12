TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.c \
    digest_match.c \
    word_match.c \
    increment.c

HEADERS += \
    digest_match.h \
    word_match.h \
    increment.h
