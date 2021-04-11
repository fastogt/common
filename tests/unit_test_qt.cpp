#include <gtest/gtest.h>

#include <QKeyEvent>

#include <common/qt/gui/shortcuts.h>

TEST(Qt, Shortcuts) {
  common::qt::gui::FastoQKeySequence seq(Qt::NoModifier, Qt::Key_Camera);
  QKeyEvent ev(QEvent::KeyPress, Qt::Key_Camera, Qt::NoModifier);

  ASSERT_TRUE(seq == &ev);

  common::qt::gui::FastoQKeySequence mod_seq(Qt::ShiftModifier, Qt::Key_O);
  QKeyEvent mod_ev(QEvent::KeyPress, Qt::Key_O, Qt::ShiftModifier);

  ASSERT_TRUE(mod_seq == &mod_ev);
  ASSERT_FALSE(seq == &mod_ev);

  QKeySequence mod_qsec("Shift+O");
  ASSERT_TRUE(mod_qsec == mod_seq);

  QKeySequence mod_qsec2("Ctrl+T");
  common::qt::gui::FastoQKeySequence mod_seq2(Qt::ControlModifier, Qt::Key_T);
  QKeyEvent mod_ev2(QEvent::KeyPress, Qt::Key_T, Qt::ControlModifier);

  ASSERT_TRUE(mod_seq2 == &mod_ev2);
  ASSERT_TRUE(mod_qsec2 == mod_seq2);
}
