# CMake generated Testfile for 
# Source directory: /home/zchcn/SE/Accounting
# Build directory: /home/zchcn/SE/Accounting/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(AccountingTestBill "/home/zchcn/SE/Accounting/bin/accounting_test_bill")
set_tests_properties(AccountingTestBill PROPERTIES  _BACKTRACE_TRIPLES "/home/zchcn/SE/Accounting/CMakeLists.txt;189;add_test;/home/zchcn/SE/Accounting/CMakeLists.txt;0;")
add_test(AccountingTestCategory "/home/zchcn/SE/Accounting/bin/accounting_test_category")
set_tests_properties(AccountingTestCategory PROPERTIES  _BACKTRACE_TRIPLES "/home/zchcn/SE/Accounting/CMakeLists.txt;190;add_test;/home/zchcn/SE/Accounting/CMakeLists.txt;0;")
add_test(AccountingTestBottomToTop "/home/zchcn/SE/Accounting/bin/accounting_test_bottom_to_top")
set_tests_properties(AccountingTestBottomToTop PROPERTIES  _BACKTRACE_TRIPLES "/home/zchcn/SE/Accounting/CMakeLists.txt;191;add_test;/home/zchcn/SE/Accounting/CMakeLists.txt;0;")
add_test(AccountingTestTopToBottom "/home/zchcn/SE/Accounting/bin/accounting_test_top_to_bottom")
set_tests_properties(AccountingTestTopToBottom PROPERTIES  _BACKTRACE_TRIPLES "/home/zchcn/SE/Accounting/CMakeLists.txt;192;add_test;/home/zchcn/SE/Accounting/CMakeLists.txt;0;")
