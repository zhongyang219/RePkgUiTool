#pragma once
#include <QApplication>
#include <QDesktopWidget>

#define DPI(x) (QApplication::desktop()->logicalDpiX() * (x) / 96)
