<?php

/**
 * @file
 * Contains Drupal\data_export_import\Form\ExportNodes.
 */

namespace Drupal\data_export_import\Form;

use Drupal\Core\Form\FormBase;
use Drupal\Core\Form\FormStateInterface;

/**
 * Implements an test form.
 */
class ExportNodes extends FormBase {

  /**
   * {@inheritdoc}
   */
  public function getFormId() {
    return 'data_export_import_export_nodes_form';
  }

  /**
   * {@inheritdoc}
   */
  public function buildForm(array $form, FormStateInterface $form_state) {

    $form['description'] = array('#markup' => 'Export all nodes to a dataset file');

    $node_types = node_type_get_types();
    foreach ($node_types as $name => $node_type) {
      $node_type_type = $name;
      $node_type_name = $node_type->get('name');

      $form['content_types'][$node_type_type] = array(
        '#type' => 'checkbox',
        '#title' => $node_type_name);
    }

    // Adds a simple submit button that refreshes the form and clears its
    // contents. This is the default behavior for forms.
    $form['actions']['#type'] = 'actions';
    $form['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => $this->t('Create dataset file'),
      '#button_type' => 'primary',
    );

    return $form;
  }

  /**
   * {@inheritdoc}
   */
  public function submitForm(array &$form, FormStateInterface $form_state) {

    data_export_import_export_nodes_to_file($form_state);

    return TRUE;
  }


  /**
   * Export the required dataset files.
   *
   * This function will look at which content types have been selected
   * for exporting to file and call a function to export thoese content
   * types.  The $dataset_files_created variable will hold the names of
   * all the dataset files which were created so it can be handed back
   * to the calling code for display to the user.
   *
   * @param array $form_state
   *   Current values held in the form.
   *
   * @return bool
   *   TRUE if all ran OK.
  */
  function data_export_import_export_nodes_to_file($form_state) {

    // See if any content types were selected - if none then exit out
    // gracefully.
    $at_least_one_content_type_is_selected_flag = FALSE;
    foreach ($form_state['values']['export_nodes']['content_types'] as $content_type => $value) {
      if ($value == 1) {
 
        $at_least_one_content_type_is_selected_flag = TRUE;
      }
    }

    if (!$at_least_one_content_type_is_selected_flag) {
      drupal_set_message(t("No content types selected."));
      return TRUE;
    }

    // Create the default directory to hold the datasets.
    $dataset_directory_parent_directory = $conf['file_public_path']. "/data_export_import";
    file_prepare_directory($dataset_directory_parent_directory, FILE_CREATE_DIRECTORY);

    $dataset_directory = variable_get('file_public_path', conf_path() . '/files') . "/data_export_import/nodes/";
    file_prepare_directory($dataset_directory, FILE_CREATE_DIRECTORY);

    // Adding in the main values to the $batch variable.
    $batch = array();
    $batch['finished'] = 'data_export_import_batch_export_nodes_finished';
    $batch['title'] = t('Exporting nodes');
    $batch['init_message'] = t('The exportation of nodes starting.');
    $batch['progress_message'] = t('Processed @current out of @total.');
    $batch['error_message'] = t('Exporting nodes has encountered an error.');
    $batch['file'] = drupal_get_path('module', 'data_export_import') . '/includes/profiles/nodes.inc';

    // We can loop through which content types need to be exported.
    // The #post array will only contain values which have been set to
    // have a value of '1'. We will build up the $batch object with the
    // required operations.
    foreach ($form_state['values']['export_nodes']['content_types'] as $content_type => $value) {

      // This should mean that the rest of this loop will not be called if the
      // content_type was not selected.
      if ($value == 0) {
        continue;
      }

      $file_name = format_date(REQUEST_TIME, 'custom', 'Ymd_His') . "_nodes_" . $content_type . ".dataset";
      $file_path_and_name = $dataset_directory . "/" . $file_name;

      // Rebuild is needed to flush out content types which may have been deleted.
      node_types_rebuild();
      $node_types = node_type_get_types();

      // Save the content type variable to the file. By serializing the
      // variable we will change it to a character based format which is
      // safe to be output to a file.  This is made safer then by being
      // base64 encoded to make sure line endings and other characters
      // do not cause issues on importing the dataset.
      $content_type_data_serialized = serialize($node_types[$content_type]);
      $content_type_data_serialized_and_encoded = base64_encode($content_type_data_serialized);
      file_unmanaged_save_data($content_type_data_serialized_and_encoded . "\n", $file_path_and_name, FILE_EXISTS_REPLACE);

      // Here we will loop through the fields in the content type and
      // look for fields which are of type 'file'.  We will then send an
      // array of these fields into the
      // data_export_import_batch_export_nodes_to_file function - this
      // will mean that the
      // data_export_import_batch_export_nodes_to_file function will
      // know which are the file fields so it can store the file data.
      $fields_of_type_file = array();
      $fields_info = field_info_instances('node', $content_type);

      foreach ($fields_info as $field_name => $field_value) {

        $field_info = field_info_field($field_name);
        $type = $field_info['type'];
  
        if ($type == 'file' || $type == 'image') {

          $fields_of_type_file[] = $field_name;
        }
      }

      // Each content type being exported to a dataset file will be run
      // as a batch to prevent timeouts.
      $batch['operations'][] = array(
        'data_export_import_batch_export_nodes_to_file',
        array(
          $content_type,
          $file_path_and_name,
          $fields_of_type_file,
        ),
      );
    }

    // This is the key function which will set the batch up to be processed.
    batch_set($batch);

    // Since we are not calling this batch processing from a form we will need to
    // request that the batch is processed.
    batch_process('admin/config/system/data_export_import/nodes');

    return TRUE;
  }

}


