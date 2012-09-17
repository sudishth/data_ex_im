<?php
/**
 * @file
 * Enables taxonomy terms to be exported and imported.
 *
 * When exporting, all if the vocabularies details will be exported
 * along with all the terms.
 *
 * When importing, all of the vocabularies on the receiving instance
 * will be checked first to check that they match the vocabularies
 * in the dataset. If there are any differences in the vocabularies
 * then they import routine will raise an error then abort.
 *
 * After updating or inserting a taxonomy term there will be a check
 * to see if the i18ntaxonomy module is installed.  If it is installed
 * then the value of the 'language' column of the {term_data} table
 * will be updated with the imported value.
 *
 * It is better that this is called after the term is updated or
 * inserted with the API functions as the hook_taxonomy() function of
 * i18ntaxonomy.module incorrectly sets the value to a default value
 * if it can't find a value in a couple of forms.
 */

/**
 * Callback function to export taxonomy terms.
 */
function data_export_import_callback_export_taxonomy_terms() {

  return drupal_get_form('data_export_import_export_taxonomy_terms_form');
}

/**
 * Function to create form to export taxonomy terms.
 */
function data_export_import_export_taxonomy_terms_form($form_state) {

  $form['export_taxonomy_terms'] = array(
    '#type' => 'fieldset',
    '#title' => t('Export taxonomy terms'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  $form['export_taxonomy_terms']['description'] = array(
    '#type' => 'item', 
    '#title' => t('Export all taxonomy terms to a dataset file'),
  );

  // Adds a simple submit button that refreshes the form and clears its
  // contents. This is the default behavior for forms.
  $form['export_taxonomy_terms']['submit'] = array(
    '#type' => 'submit', 
    '#value' => 'Create dataset file',
  );

  return $form;
}

/**
 * Function to process form to export taxonomy terms.
 */
function data_export_import_export_taxonomy_terms_form_submit($form, &$form_state) {

  // This module has not yet been extended to correctly handle data
  // which has internationalization (i18n) enabled.
  if (module_exists('i18n')) {

    drupal_set_message(t("The data export functionality is not currently compatible with internationalization (i18n)."), 'error');
    return TRUE;
  }

  // This is the function which will carry out the exporting of the
  // taxonomy terms to a dataset file.
  $dataset_file_created = data_export_import_export_taxonomy_terms_to_file();

  drupal_set_message(t("The following dataset file was created:"));
  drupal_set_message(check_plain($dataset_file_created));

  return TRUE;

  /* Original code.

  // This module has not yet been extended to correctly handle data
  // which has internationalization (i18n) enabled.
  if (module_exists('i18n')) {

    drupal_set_message(t("The data export functionality is not currently compatible with internationalization (i18n)."), 'error');
    return TRUE;
  }

  data_export_import_taxonomy_terms_export_to_file($form_state, $dataset_files_created);

  drupal_set_message(t("The following dataset files were created:"));

  foreach ($dataset_files_created as $dataset_file_name) {
    drupal_set_message(check_plain($dataset_file_name));
  }

  return TRUE;
  */


}


// function to export terms needed


/**
 * Export the required dataset file.
 */
/*
function data_export_import_taxonomy_terms_export_to_file($form_state, &$dataset_files_created) {

  if ($form_state['clicked_button']['#post']['taxonomy_terms']['taxonomy_terms'] == 1) {
    $dataset_files_created[] = data_export_import_export_taxonomy_terms();
  }
}
*/

/**
 * Exports detailed data about the vocabularies and its terms.
 *
 * This function is going to use the API to extract the relevant
 * data. This data is then going to be output to a data file.
 *
 * The first element in the array will be an array of the
 * vocabularies and the vocabulary details. This will be used
 * during the import to check that there are the correct
 * vocabularies in place which are ready to receive the terms being
 * imported.
 *
 * The second element will contain details of all the terms which
 * are in each vocabulary. These terms will be used to ensure that
 * the receiving instance ends up with a matching set of terms.
 *
 * @return string
 *   The filename of the datafile which was created.
 */
function data_export_import_export_taxonomy_terms_to_file() {

  // This will be the main array which will hold the data which will
  // be output to the dataset file.
  $dataset = array();

  // Add the vocabularies data the the array which will exported.
  $vocabularies_array = taxonomy_get_vocabularies();
  $dataset['vocabularies'] = $vocabularies_array;

  // Add the vocabulary terms to the main array.
  // We will loop through the vocabularies array and use the vid to
  // then get the list of terms in each vocabulary.
  foreach ($vocabularies_array as $vocabulary) {

    $vocabulary_id = $vocabulary->vid;

    // Here we will get the terms related to the vocabulary - then
    // we'll add on the synonyms and add on the related terms - then
    // we'll add the array on terms to the main array.
    $terms_array = data_export_import_taxonomy_get_tree_with_reset($vocabulary->vid, 0, -1, NULL, TRUE);

    // The $list_of_terms will be used to dedupe the terms returned by
    // data_export_import_taxonomy_get_tree_with_reset() which outputs
    // terms twice if they have two parents.
    $list_of_terms = array();
    foreach ($terms_array as $array_key => $term) {

      // If the term has been exported already then we don't want to
      // export it again. So we'll remove the term from the array.
      if (in_array($term->tid, $list_of_terms)) {
        unset($terms_array[$array_key]);
      }
      else {

        // Here we are going to add all the term id to an array to
        // keep a list of all terms currently output. This is because
        // when terms have more than one parent they are output twice.
        // What we are going to do is to check to see if the term has
        // been output already - and if so then not re-output it
        // again.
        $list_of_terms[] = $term->tid;

        // Here we add the synonyms to the term object being
        // exported.
        $synonyms_array = taxonomy_get_synonyms($term->tid);

        // Sort the array to make it consistent with a later call to
        // this function. When they are extracted using the API
        // function  they are not sorted in any particular order.
        asort($synonyms_array);

        $term->synonyms = $synonyms_array;

        // Add in the related terms.
        $related_terms = taxonomy_get_related($term->tid);

        $array_of_related_term_tids = array();

        foreach ($related_terms as $related_term) {
          $array_of_related_term_tids[] = $related_term->tid;
        }

        $term->relations = $array_of_related_term_tids;
      }
    }

    // Before we add the terms to the data being output we are going
    // to set the array key value to be the term id. Currently the
    // array of terms have a sequential key: 0,1,2,3 etc - these will
    // be changed to be the term id value. As the terms id's are
    // unique then there will be no conflict - and this makes the
    // array more useful later on.
    $terms_array_with_tid_as_key = array();
    foreach ($terms_array as $term) {
      $terms_array_with_tid_as_key[$term->tid] = $term;
    }

    // Now add the array of terms to the dataset array and use the vid
    // as the key. Using the vid as key makes it easier to search this
    // array during the import.
    $dataset['vocabulary_terms'][$vocabulary_id] = $terms_array_with_tid_as_key;
  }

  // Here we will serialize the array to convert it to a string which
  // can then be output to a file.
  $dataset_serialized = serialize($dataset);

  // Create the default directory to hold the datasets.
  $dataset_directory_parent_directory = file_directory_path() . "/data_export_import";
  file_check_directory($dataset_directory_parent_directory, $mode = FILE_CREATE_DIRECTORY);

  $dataset_directory = file_directory_path() . "/data_export_import/taxonomy_terms/";
  file_check_directory($dataset_directory, $mode = FILE_CREATE_DIRECTORY);

  // Save the string as a file. This is the dataset data file.

  //  $file_name = format_date(time(), 'custom', 'Ymd_His') . "_taxonomy_terms.dataset";
  $file_name = format_date(REQUEST_TIME, 'custom', 'Ymd_His') . "_taxonomy_terms.dataset";
  $file_path_and_name = $dataset_directory . "/" . $file_name;

  file_save_data($dataset_serialized, $file_path_and_name, FILE_EXISTS_REPLACE);

  return $file_name;
}
































/**
 * Callback function to import taxonomy terms.
 */
function data_export_import_callback_import_taxonomy_terms() {

  return drupal_get_form('data_export_import_import_taxonomy_terms_form');
}

/**
 * Function to create form to import taxonomy terms.
 */
function data_export_import_import_taxonomy_terms_form($form_state) {

  $form = array();

  $form['import_taxonomy_terms'] = array(
    '#type' => 'fieldset',
    '#title' => t('Import taxonomy terms'),
    '#collapsible' => FALSE,
    '#collapsed' => FALSE,
  );

  // Get the contents of the dataset directory and create a list of
  // links to dataset files.
  $directory = variable_get('file_public_path', conf_path() . '/files') . "/data_export_import/taxonomy_terms";

  // $mask = '.dataset';
  $mask = '/.dataset/';

  $files = file_scan_directory($directory, $mask);

  // Sort them by the filename which is used as the key.  Since the
  // files are named using datetime stamps they will be listed in
  // date/time order.
  ksort($files);

  $options = array();
  $options['none'] = "None";

  foreach ($files as $file) {

    $options[$file->filename] = check_plain($file->filename);
  }

  $form['import_taxonomy_terms']['dataset_file'] = array(
    '#type' => 'radios',
    '#title' => t('Select file to import - NB You might want to switch off account activation email notification at /admin/config/people/accounts.'),
    '#default_value' => 'none',
    '#options' => $options,
  );

  $form['import_taxonomy_terms']['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Import dataset files',
  );

  return $form;
}

/**
 * Function to process form to import terms.
 */
function data_export_import_import_taxonomy_terms_form_submit($form, &$form_state) {

  if ($form_state['values']['dataset_file'] != 'none') {

    $result = data_export_import_import_taxonomy_terms($form_state['values']['dataset_file']);

    if ($result) {
      drupal_set_message(t('The Taxonomy Terms dataset file %dataset_file was imported.', array('%dataset_file' => $form_state['values']['dataset_file'])));
    }
    else {
      drupal_set_message(t('The Taxonomy Terms dataset file %dataset_file was not imported.', array('%dataset_file' => $form_state['values']['dataset_file'])), 'error');
    }
  }
}


//function data_export_import_import_terms($file) {
