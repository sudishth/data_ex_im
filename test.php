<?php

/**
 * @file
 * This file is used to run various functions etc from the command line.
 *
 * As we are developing batch operations it is useful to be able to
 * test functions from the command line.  For example, this file can
 * be called via drush using:
 * drush @example_site php-script /home/username/projects/example_project/web/trunk/sites/all/modules/data_export_import/test.php
 */

require_once 'includes/taxonomy_terms.inc';

// Test the export.
//$dataset_file = data_export_import_export_taxonomy_terms();
//echo "dataset file: ".$dataset_file."\n";

// Test the import.
data_export_import_import_taxonomy_terms("20111027_182140_taxonomy_terms.dataset");

return;
