<?php

/**
 * @file
 * This file is used to run various functions etc from the command line.
 *
 * As we are developing batch operations it is useful to be able to
 * test functions from the command line.  For example, this file can
 * be called via drush using:
 * drush @example_site php-script /path/to/test.php
 */

require_once 'includes/profiles/articles.inc';
require_once 'includes/profiles/pages.inc';
require_once 'includes/profiles/taxonomy_terms.inc';

// Test the export.
// $dataset_file = data_export_import_export_pages();
// echo "dataset file: " . $dataset_file . "\n";
// *
// Test the import.
// data_export_import_import_taxonomy_terms("20111116_114401_pages.dataset");
data_export_import_import_pages("20111122_124839_pages.dataset");

// XXX echo "\n\n<-----START OF FILE IMPORT--->\n\n";
// data_export_import_import_pages($dataset_file);
// *
return;

