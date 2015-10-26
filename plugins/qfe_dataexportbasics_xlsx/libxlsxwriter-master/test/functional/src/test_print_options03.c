/*****************************************************************************
 * Test cases for libxlsxwriter.
 *
 * Test to compare output against Excel files.
 *
 * Copyright 2014-2015, John McNamara, jmcnamara@cpan.org
 *
 */

#include "xlsxwriter.h"

int main() {

    lxw_workbook  *workbook  = new_workbook("test_print_options03.xlsx");
    lxw_worksheet *worksheet = workbook_add_worksheet(workbook, NULL);

    worksheet_set_paper(worksheet, 9);
    worksheet->vertical_dpi = 200;

    worksheet_center_vertically(worksheet);

    worksheet_write_string(worksheet, CELL("A1"), "Foo" , NULL);

    return workbook_close(workbook);
}
