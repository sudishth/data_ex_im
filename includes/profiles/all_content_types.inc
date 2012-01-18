<?php
/**
 * @file
 * Provides functions to enable nodes to be exported and imported.
 *
 * When importing the content type will need to be checked to make
 * sure that the nodes can be imported with all their data.
 */

/**
 * Add fieldset to the data export form.
 */
function _all_content_types_add_fieldset_to_export_form(&$form) {

  $form['all_content_types'] = array(
          '#type' => 'fieldset',
          '#title' => 'All content types.',
          '#collapsible' => TRUE,
          '#collapsed' => FALSE, );

  $node_types = node_get_types();

  foreach ($node_types as $node_type) {

    $node_type_type = $node_type->type;
    $node_type_name = $node_type->name;

    $form['all_content_types'][$node_type_type] = array(
            '#type' => 'checkbox',
            '#title' => $node_type_name, );
  }
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
 * @return
 *   TRUE if all ran OK.
 */
function _all_content_types_export_to_file($form_state, &$dataset_files_created) {

  /*
  $string_to_output = var_export($form_state, TRUE);
  drupal_set_message("<pre>" . $string_to_output . "</pre>");
  */

  // See if any content types were selected - if none then exist out
  // gracefully.
  if (!isset($form_state['clicked_button']['#post']['all_content_types'])) {
    drupal_set_message("No content types selected.");
    return TRUE;
  }

  // We can loop through which content types need to be exported.
  // The #post array will only contain values which have been set to
  // have a value of '1'.
  foreach ($form_state['clicked_button']['#post']['all_content_types'] as $content_type => $value) {

    $dataset_files_created[] = data_export_import_export_nodes_for_a_content_type($content_type);
  }

  return TRUE;
}

/**
 * Exports detailed node data.
 *
 * This function is going to use the API to extract the relevant
 * data. This data is then going to be output to a data file.
 *
 * @return
 *   The filename of the datafile which was created.
 */
function data_export_import_export_nodes_for_a_content_type($content_type) {

  // This will be the main array which will hold the data which will
  // be output to the dataset file.
  $dataset = array();

  // $node_type = node_get_types('type', 'page');

  // Here we are getting all content types and selecting just the one
  // we need - this seems to give more consistent results than just
  // extracting the single node object.
  $node_types = node_get_types();

  // Add the content type details to the array.
  // We are going to serialize the object here as it can then be
  // unserialized on the import and checked.  The normal
  // serialize/unserialize should cover this but the objects would not
  // be counted as matching.  Possibly related to:
  // https://bugs.php.net/bug.php?id=48016
  $dataset['content_type'] = serialize($node_types[$content_type]);

  // Get the node objects and add them to the array.
  $query = db_rewrite_sql("SELECT nid FROM {node} n WHERE type = '%s'", 'n');

  // $results = db_query_range($query, $type, $offset, $limit);
  $results = db_query($query, $content_type);

  while ($nid = db_result($results)) {

    // Load the node and then save the node to the array which will be
    // exported to the file.
    $node = node_load($nid);
    //    $dataset['nodes'][$node->nid] = $node;

    // Here we will deal with any files attached to the node if the
    // upload module is being used.  What we are going to do is to
    // extract the file, convert it to a character encoding and then
    // attach it to the files array on the node.
    if (module_exists('upload')) {

      if (!empty($node->files) && is_array($node->files)) {
        drupal_set_message('We have some files uploaded.');
//        dvm($node);

        // Loop through the files.
        foreach ($node->files as $attached_file_key => $attached_file) {

          // Get the file into a safe character based format and then
          // attach it to the array.
          $export_data = base64_encode(file_get_contents($attached_file->filepath));
          $node->upload_files[$attached_file_key]->data_export_import_file_data = $export_data;

          drupal_set_message($attached_file->filepath);
          // drupal_set_message($export_data);
        }
      }
    }

//    dvm($node);
    $dataset['nodes'][$node->nid] = $node;



//    dvm($node->field_image_field[0]['filepath']);


    // Here we are looking for all the file data fields.
    foreach($node as $node_field_key => $node_field_value) {

//      if (is_array($node_field_value) && ($node_field_key != 'cck_files')) {
      if (is_array($node_field_value)) {

        foreach($node_field_value as $file_detail_key => $file_details_value) {

          // This looks for the correct type of value and if there is
          // a setting for fid, filename, filepath set then we will assume this
          // is a field of files attached via CCK.
          if (is_array($file_details_value) && isset($file_details_value['fid']) && isset($file_details_value['filename']) &&isset($file_details_value['filepath'])) {

            $i = 123;
            // $node->field_image1[$file_detail_key]['cck_file_data'] = "abcdefghij";

            // Get the file data in a serialized format and attach this as data
            // to the correct place on the $node object.

            $file_data = base64_encode(file_get_contents($file_details_value['filepath']));

//            $node->{$node_field_key}[$file_detail_key]['cck_file_data'] = "abcdefghij - " . $file_details_value['filename'];
            $node->{$node_field_key}[$file_detail_key]['cck_file_data'] = $file_data;

            // Here we are going to add the file data to the array - and save it
            // on to the node object.

            // NB NB NB
            // HERE HERE HERE
            // Here we need to extract the file and serialize it as per the 'files' section.

            /*
            $file_details_value['cck_file_data'] = "123123123";
            $node->cck_files[] = $file_details_value;
            */
            $k = 321;


//            dvm($file_details_value['filename']);
//           dvm($file_details_value['filepath']);
          }
        }
      }
    }




  }

// loop through all nodes.

// loop through all fields

// certain fields contain arrays - if the content of the field is an array then loop through it.

// look for the values filename and filepath.



  // Here we will serialize the array to convert it to a string which
  // can then be output to a file.
  $dataset_serialized = serialize($dataset);

  // Create the default directory to hold the datasets.
  $dataset_directory_parent_directory = file_directory_path() . "/data_export_import";
  file_check_directory($dataset_directory_parent_directory, $mode = FILE_CREATE_DIRECTORY);

  $dataset_directory = file_directory_path() . "/data_export_import/all_content_types";
  file_check_directory($dataset_directory, $mode = FILE_CREATE_DIRECTORY);

  // Save the string as a file. This is the dataset data file.
  $file_name = format_date(time(), 'custom', 'Ymd_His') . "_all_content_types_". $content_type .".dataset";
  $file_path_and_name = $dataset_directory . "/" . $file_name;

  file_save_data($dataset_serialized, $file_path_and_name, FILE_EXISTS_REPLACE);

  return $file_name;
}

/**
 * Make current nodes match the nodes from a dataset file.
 *
 * The purpose of this function is to effectively 'import' the nodes
 * stored in a dataset file.
 *
 * NB - When this import has finished the nodes in the receiving
 * instance should be an exact match with the nodes in the imported
 * dataset file.  Think in terms of rsync with the '--delete'
 * option. This means that as well as importing new nodes we need to
 * delete nodes from the receiving instance which are not in the
 * imported dataset.
 *
 * This synchronisation will be carried out by two passes.
 *
 * First we will loop through the nodes in the receiving instance  and
 * check against the the imported dataset. If the node exists in the
 * dataset then it will be updated in the receiving instance.  If it
 * doesn't exist in the dataset then it will be deleted from the
 * receiving Drupal instance.
 *
 * The second pass will be a loop through the dataset - any terms
 * which are in the dataset but are not in the receiving Drupal
 * instance will be created.
 *
 * This will effectively mean that the terms have been sychronised
 * completely.
 *
 * NB - There is some deeper logic here we need to be aware of.  The
 * node ID's need to match exactly otherwise the related items will
 * not match.  So when the new nodes are created they will need to
 * have their old ID's set to match exactly - again due to the related
 * terms.
 *
 * @param string $file
 *   The dataset file which is being imported.
 *
 * @return bool
 *   TRUE if the import process ran without errors.
 */

// HERE HERE HERE - change to import nodes/all_content_types.

function data_export_import_import_all_content_types($file) {

  // @todo Check that the users exist and match.
  // *
  // Load the dataset file into a variable.
  $file_content = file_get_contents(file_directory_path() . "/data_export_import/all_content_types/" . $file);

  // Decode the serialized data and store it in an array of objects.
  $file_content_as_array = unserialize($file_content);
//  dvm($file_content_as_array);
  // dvm($file_content_as_array['nodes'][1]->field_audio[0]['filepath']);

  /*
  foreach($file_content_as_array['nodes'] as $node_from_file) {

    $a == 1;
// loop through all nodes.

// loop through all fields

// certain fields contain arrays - if the content of the field is an array then loop through it.

// look for the values filename and filepath.
  }
  */


  // Here we need to get the content type of the nodes being imported.
  $node_content_type_object = unserialize($file_content_as_array['content_type']);
//  dvm($node_content_type_object);
  $node_content_type = $node_content_type_object->type;

  // Check that the content type is an exact match between the sending
  // instance and the receiving instance.
  // This has been individually serialized due to the objects not
  // being declared as matching when not serailized/unserialized
  // first.  Possibly related to:
  // https://bugs.php.net/bug.php?id=48016
  $node_types = node_get_types();
  if (unserialize($file_content_as_array['content_type']) != $node_types[$node_content_type]) {

    drupal_set_message(t("When the dataset was created the node content type was different from this site's node content type. Please manually compare the content type: %content_type", array('%content_type' => $dataset->content_type)), 'error');
    return;
  }

  // Loop through all the existing nodes - and if they don't exist in
  // the dataset then delete them.
  $query = db_rewrite_sql("SELECT nid FROM {node} n WHERE type = '%s'", 'n');
  $results = db_query($query, $node_content_type);

  while ($nid = db_result($results)) {

    if (!isset($file_content_as_array['nodes'][$nid])) {

      // Delete node as it does not exist in the dataset being
      // imported.
      node_delete($nid);
    }
  }

  // Here we will pick up if the file contains no nodes.
  if (empty($file_content_as_array['nodes'])) {
    return;
  }

  // Loop through the nodes in the dataset.
  // Update nodes which are different in the dataset and create nodes
  // which don't exist in the dataset.
  foreach ($file_content_as_array['nodes'] as $dataset_node) {

    // Find if there is an existing node and see if it matches what is
    // in the dataset.
    // NB node_load() returns FALSE if it can't find a node.
    $existing_node = node_load($dataset_node->nid);

    // If node_load returns FALSE then it was not able to find a node
    // with the nid - therefore a new node needs to be created.
    // Otherwise the existing node may need to be updated with the
    // data from the dataset.
    if ($existing_node == FALSE) {

      // Saving data for a new node is relatively straight forward.  We just
      // call a modified version of the node_save() function.  The difficult
      // part is dealing with attached files and files uploaded via CCK fields.

      // First we will remove the files data from the node variable and save
      // the node.  Then we will add the files in a two stage process.
      // 1. Emulate the uploading of a file.
      // 2. Emulate saving the file as an attachment.


      // Use a modified node_save() function which allows the node to
      // be saved with it's existing nid.
      //
      // Loop through all the CCK fields create the required files/ and {files}


// Here we will loop through the fields looking for CCK fields which contain
// file data.

      foreach($dataset_node as $node_field_key => $node_field_value) {

        if (is_array($node_field_value)) {

          foreach($node_field_value as $file_detail_key => $file_details_value) {

            // This looks for the correct type of value and if there is
            // a setting fid/filename/filepath set then we will assume this is a
            //  field of attached CCK files.
            if (is_array($file_details_value) && isset($file_details_value['fid']) && isset($file_details_value['filename']) &&isset($file_details_value['filepath'])) {


              $a = 1;

              if (isset($file_details_value['cck_file_data'])) {

                $directory = file_create_path(dirname($file_details_value['filepath']));

                if (file_check_directory($directory, TRUE)) {
                  if (file_put_contents($file_details_value['filepath'], base64_decode($file_details_value['cck_file_data']))) {
                    drupal_write_record('files', $file_details_value);

                    // As we've just saved the file it will probably have been
                    // given a new fid.  We are not going to force the file to
                    // be saved with the same ID as in the dataset as there may
                    // be another file which is using the ID value.  We need to 
                    // get this new ID value and put that into the node data so
                    // the node links to the correct file.
                    $dataset_node->{$node_field_key}[$file_detail_key]['fid'] = $file_details_value['fid'];


                    $b = 1;
                   }
                }

              }
              // dvm($file_details_value['filename']);
              // dvm($file_details_value['filepath']);
            }
          }
        }
      }


      data_export_import_node_insert_with_defined_nid($dataset_node);


      // The CCK fields should have been filled in correctly.  We now need to
      // create the actual file from the data in the file and create the
      // correcponding record in the {files} table.


      /*

      if (isset($file->node_export_file_data)) {
      }
    $directory = file_create_path(dirname($file->filepath));

    if (file_check_directory($directory, TRUE)) {
      if (file_put_contents($file->filepath, base64_decode($file->node_export_file_data))) {
        drupal_write_record('files', $file);
      }
    }
  }

      */

    }

    else {

      // If the nodes don't match then update the existing node with
      // the data from the dataset.
      // Certain fields can be unset because they would be expected to
      // be different.
      // We will store the existing dataset node because it will be
      // needed when updating the node.
      $existing_node_vid = $existing_node->vid;

      // The node may have had new revisions created since the dataset
      // was created so we'll unset all the data about revisions.
      unset($dataset_node->vid);
      unset($existing_node->vid);
      unset($dataset_node->log);
      unset($existing_node->log);
      unset($dataset_node->revision_timestamp);
      unset($existing_node->revision_timestamp);

      // The changed date for the existing node could easily be
      // different from the dataset node - even from a previous import
      // of the dataset - or say a user saved the node without any changes.
      unset($dataset_node->changed);
      unset($existing_node->changed);

      if ($dataset_node != $existing_node) {

        // Create a new revision. The dataset node may be from several
        // revisions back - and we don't want to reset back to a
        // previous revision - and we don't want to overwrite the
        // current revision - so creating a new revision is the best
        // option. The only other option is to delete the node (which
        // would delete all revisions) and then re-create it - but
        // this would lose all the revision history.
        $dataset_node->vid = $existing_node_vid;
        $dataset_node->revision = 1;
        $dataset_node->log = "Imported from dataset file: " . $file;

        // Here we will save the files attached to the node via CCK fields and
        // update the fid values stored in the $dataset_node.

        // xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

      foreach($dataset_node as $node_field_key => $node_field_value) {

        if (is_array($node_field_value)) {

          foreach($node_field_value as $file_detail_key => $file_details_value) {

            // This looks for the correct type of value and if there is
            // a setting fid/filename/filepath set then we will assume this is a
            //  field of attached CCK files.
            if (is_array($file_details_value) && isset($file_details_value['fid']) && isset($file_details_value['filename']) &&isset($file_details_value['filepath'])) {


              $a = 1;

              if (isset($file_details_value['cck_file_data'])) {

                $directory = file_create_path(dirname($file_details_value['filepath']));

                if (file_check_directory($directory, TRUE)) {
                  if (file_put_contents($file_details_value['filepath'], base64_decode($file_details_value['cck_file_data']))) {
                    drupal_write_record('files', $file_details_value);

                    // As we've just saved the file it will probably have been
                    // given a new fid.  We are not going to force the file to
                    // be saved with the same ID as in the dataset as there may
                    // be another file which is using the ID value.  We need to
                    // get this new ID value and put that into the node data so
                    // the node links to the correct file.
                    $dataset_node->{$node_field_key}[$file_detail_key]['fid'] = $file_details_value['fid'];


                    $b = 1;
                   }
                }

              }
              // dvm($file_details_value['filename']);
              // dvm($file_details_value['filepath']);
            }
          }
        }
      }



// yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy




        node_save($dataset_node);
      }
    }
  }
}


/**
 * Implements saving a node with a defined nid value.
 *
 * This function is basically an override of the standard node_save()
 * function.
 */
function data_export_import_node_insert_with_defined_nid(&$node) {

  // Here we will remove the nid from the object to trick the rest of
  // the function into thinking it is dealing with a normal insert.
  // We will then re-introduce the nid when needed.
  $defined_nid = $node->nid;
  unset($node->nid);

  // Let modules modify the node before it is saved to the database.
  node_invoke_nodeapi($node, 'presave');
  global $user;

  // Insert a new node.
  $node->is_new = empty($node->nid);

  if ($node->is_new || !empty($node->revision)) {
    // When inserting a node, $node->log must be set because
    // {node_revisions}.log does not (and cannot) have a default
    // value.  If the user does not have permission to create
    // revisions, however, the form will not contain an element for
    // log so $node->log will be unset at this point.
    if (!isset($node->log)) {
      $node->log = '';
    }
  }

  // For the same reasons, make sure we have $node->teaser and
  // $node->body set.
  if (!isset($node->teaser)) {
    $node->teaser = '';
  }
  if (!isset($node->body)) {
    $node->body = '';
  }

  $time = time();
  if (empty($node->created)) {
    $node->created = $time;
  }

  // The changed timestamp is always updated for bookkeeping purposes
  // (revisions, searching, ...)
  $node->changed = $time;

  $node->timestamp = $time;
  $node->format = isset($node->format) ? $node->format : FILTER_FORMAT_DEFAULT;

  // Generate the node table query and the node_revisions table query.
  if ($node->is_new) {
    _node_save_revision($node, $user->uid);

    // Call our modified function after putting the nid back into the
    // object.
    $node->nid = $defined_nid;
    data_export_import_drupal_write_record_via_insert_with_defined_id('node', $node);

    db_query('UPDATE {node_revisions} SET nid = %d WHERE vid = %d', $node->nid, $node->vid);
    $op = 'insert';
  }

  // Call the node specific callback (if any).
  node_invoke($node, $op);
  node_invoke_nodeapi($node, $op);

  // Update the node access table for this node.
  node_access_acquire_grants($node);

  // Clear the page and block caches.
  cache_clear_all();
}

/**
 * Implements inserting a record with a defined id value.
 *
 * Overrides the standard drupal_write_record() function.
 */
function data_export_import_drupal_write_record_via_insert_with_defined_id($table, &$object, $update = array()) {

  // Standardize $update to an array.
  if (is_string($update)) {
    $update = array($update);
  }

  $schema = drupal_get_schema($table);
  if (empty($schema)) {
    return FALSE;
  }

  // Convert to an object if needed.
  if (is_array($object)) {
    $object = (object) $object;
    $array = TRUE;
  }
  else {
    $array = FALSE;
  }

  $fields = $defs = $values = $serials = $placeholders = array();

  // Go through our schema, build SQL, and when inserting, fill in defaults for
  // fields that are not set.
  foreach ($schema['fields'] as $field => $info) {

    // For inserts, populate defaults from Schema if not already provided.
    if (!isset($object->$field) && !count($update) && isset($info['default'])) {
      $object->$field = $info['default'];
    }

    // Build arrays for the fields, placeholders, and values in our query.
    if (isset($object->$field)) {
      $fields[] = $field;
      $placeholders[] = db_type_placeholder($info['type']);

      if (empty($info['serialize'])) {
        $values[] = $object->$field;
      }
      else {
        $values[] = serialize($object->$field);
      }
    }
  }

  // Build the SQL.
  $query = '';
  if (!count($update)) {
    $query = "INSERT INTO {" . $table . "} (" . implode(', ', $fields) . ') VALUES (' . implode(', ', $placeholders) . ')';
    $return = SAVED_NEW;
  }

  // Execute the SQL.
  if (!db_query($query, $values)) {
    $return = FALSE;
  }

  // If we began with an array, convert back so we don't surprise the caller.
  if ($array) {
    $object = (array) $object;
  }

  return $return;
}




/**
 * Exports detailed node data.
 *
 * This function is going to use the API to extract the relevant
 * data. This data is then going to be output to a data file.
 *
 * @return
 *   The filename of the datafile which was created.
 */
/*
function data_export_import_export_all_content_types() {

  $debug = FALSE;

  // This will be the main array which will hold the data which will
  // be output to the dataset file.
  $dataset = array();

  $dataset[] = "all content types exported data nodes etc";

  // Here we will serialize the array to convert it to a string which
  // can then be output to a file.
  $dataset_serialized = serialize($dataset);

  // Create the default directory to hold the datasets.
  $dataset_directory_parent_directory = file_directory_path() . "/data_export_import";
  file_check_directory($dataset_directory_parent_directory, $mode = FILE_CREATE_DIRECTORY);

  // Create the default directory to hold the datasets.
  $dataset_directory = file_directory_path() . "/data_export_import/all_content_types";
  file_check_directory($dataset_directory, $mode = FILE_CREATE_DIRECTORY);

  // Save the string as a file. This is the dataset data file.
  $file_name = format_date(time(), 'custom', 'Ymd_His') . "_all_content_types.dataset";
  $file_path_and_name = $dataset_directory . "/" . $file_name;

  file_save_data($dataset_serialized, $file_path_and_name, FILE_EXISTS_REPLACE);

  return $file_name;
}
*/
