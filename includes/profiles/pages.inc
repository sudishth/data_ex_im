<?php

/**
 * @file
 * Provides functions to enable pages to be exported and imported.
 *
 * When importing the content type will need to be checked to make
 * sure that the pages can be imported with all their data.
 */

/**
 * Exports detailed data about the pages.
 *
 * This function is going to use the API to extract the relevant
 * data. This data is then going to be output to a data file.
 *
 * @return
 *   The filename of the datafile which was created.
 */
function data_export_import_export_pages() {

  $debug = FALSE;

  // This will be the main array which will hold the data which will
  // be output to the dataset file.
  $dataset = array();

  // $node_type = node_get_types('type', 'page');
  $node_types = node_get_types();
  if ($debug) {
    echo "node_types:\n";
    var_export($node_types['page']);
  }

  // Add the content type details to the array.
  // We are going to serialize the object here as it can then be
  // unserialized on the import and checked.  The normal
  // serialize/unserialize should cover this but the objects would not
  // be counted as matching.  Possibly related to:
  // https://bugs.php.net/bug.php?id=48016
  $dataset['content_type'] = serialize($node_types['page']);

  $type = 'page';

  // Get the node objects and add them to the array.
  $query = db_rewrite_sql("SELECT nid FROM {node} n WHERE type = '%s'", 'n');

  // $results = db_query_range($query, $type, $offset, $limit);
  $results = db_query($query, $type);

  while ($nid = db_result($results)) {
    $node = node_load($nid);
    $dataset['nodes'][$node->nid] = $node;
  }

  if ($debug) {
    echo "Dataset being exported ------------------------------->:\n";
    var_export($dataset);
  }

  // Here we will serialize the array to convert it to a string which
  // can then be output to a file.
  $dataset_serialized = serialize($dataset);

  if ($debug) {
    echo "Dataset after being serialized ------------------------------->:\n";
    var_export($dataset_serialized);
  }

  // Create the default directory to hold the datasets.
  $dataset_directory = file_directory_path() . "/data_export_import/pages";
  file_check_directory($dataset_directory, $mode = FILE_CREATE_DIRECTORY);

  // Save the string as a file. This is the dataset data file.
  $file_name = format_date(time(), 'custom', 'Ymd_His') . "_pages.dataset";
  $file_path_and_name = $dataset_directory . "/" . $file_name;

  file_save_data($dataset_serialized, $file_path_and_name, FILE_EXISTS_REPLACE);

  return $file_name;
}

/**
 * Make pages match the pages from a dataset file.
 *
 * The purpose of this function is to effectively 'import' the pages
 * stored in a dataset file.
 *
 * NB - When this import has finished the pages in the receiving
 * instance should be an exact match with the pagess in the imported
 * dataset file.  Think in terms of rsync with the '--delete'
 * option. This means that as well as importing new pages we need to
 * delete pages from the receiving instance which are not in the
 * imported dataset.
 *
 * This synchronisation will be carried out by two passes.
 *
 * First we will loop through the pages in the receiving instance  and
 * check against the the imported dataset. If the page exists in the
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
 * page ID's need to match exactly otherwise the related items will
 * not match.  So when the new pages are created they will need to
 * have their old ID's set to match exactly - again due to the related
 * terms.
 *
 * @param $file
 *   The dataset file which is being imported.
 *
 * @return
 *   TRUE if the import process ran without errors.
 */
function data_export_import_import_pages($file) {

  $debug = TRUE;

  // Need to add a check that the file exists and handle the error.
  // *
  // XXX if (!file_exists($file)) {
  // echo "file does not exist: " . $file."\n";
  // }
  // *
  // Load the dataset file into a variable.
  $file_content = file_get_contents("sites/default/files/data_export_import/pages/" . $file);

  // Decode the serialized data and store it in an array of objects.
  $file_content_as_array = unserialize($file_content);

  if ($debug) {
    echo "----------------->file content as array:\n";
    var_export($file_content_as_array);
  }

  // Check that the content type is an exact match between the sending
  // instance and the receiving instance.
  // This has been individually serialized due to the objects not
  // being declared as matching when not serailized/unserialized
  // first.  Possibly related to:
  // https://bugs.php.net/bug.php?id=48016
  $node_types = node_get_types();
  if (unserialize($file_content_as_array['content_type']) != $node_types['page']) {

    if ($debug) {
      echo "\nContent types do not match.---------------> \n";
      echo "content_type from array:\n";
      var_export(unserialize($file_content_as_array['content_types']));
      echo "\n";
    }

    if ($debug) {
      echo "content_type from api call:\n";
      var_export($node_types);
      echo "\n";
    }

    drupal_set_message(check_plain("ERROR - When the dataset was created the page content type was different from this site's page content type."));
    return;
  }

  if ($debug) {
    echo "\nContent types match.---------------> \n";
  }

  // Loop through all the existing nodes - and if they don't exist in
  // the dataset then delete them.
  $type = 'page';

  // Get the node objects.
  $query = db_rewrite_sql("SELECT nid FROM {node} n WHERE type = '%s'", 'n');
  $results = db_query($query, $type);

  while ($nid = db_result($results)) {

    if ($debug) {
      echo "\nNID of existing node--------------->:" . $nid . "\n";
    }

    if (!isset($file_content_as_array['nodes'][$nid])) {
      if ($debug) {
        echo "Array does not hold a value - therefore this node should be deleted.";
      }
      // Delete node as it does not exist in the dataset being
      // imported.
      node_delete($nid);
    }
  }

  // Loop through the nodes in the dataset.
  // Update nodes which are different in the dataset and create nodes
  // which don't exist in the dataset.
  foreach ($file_content_as_array['nodes'] as $dataset_node) {
    echo "\n\nPage--------------------------------------->:\n";

    if ($debug) {
      echo"\ndataset_node--------------------------------->:\n";
      var_export($dataset_node);
    }

    // Find if there is an existing node and see if it matches what is
    // in the dataset.
    // NB node_load() returns FALSE if it can't find a node.
    $existing_node = node_load($dataset_node->nid);

    if ($debug) {
      echo"\nexisting_node----------------------->:\n";
      var_export($existing_node);
    }

    // Check if there is existing node with the same nid as the
    // dataset node.
    // *
    // @todo Double check that a node with the nid of $dataset_node->nid
    // exists just in case the node_load() function returns FALSE for
    // another reason other than the node not existing.
    // *
    if ($existing_node == FALSE) {

      // Create a new node from the node in the dataset.
      // *
      // HERE HERE HERE
      // Use a modified node_save() function.
      data_export_import_node_insert_with_defined_nid($dataset_node);

    }

    else {

      // If the nodes don't match then update the existing node with
      // the data from the dataset.
      if ($dataset_node != $existing_node) {

        if ($debug) {
          echo"\n need to update the existing node with ID--------------------------------------->:" . $dataset_node->nid . "\n";
        }

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

  // No longer needed.
  // elseif (empty($node->log)) {
  // When updating a node, however, avoid clobbering an existing
  // log entry with an empty one.
  // unset($node->log);
  // }
  // *
  // For the same reasons, make sure we have $node->teaser and
  // $node->body set.
  if (!isset($node->teaser)) {
    $node->teaser = '';
  }
  if (!isset($node->body)) {
    $node->body = '';
  }

  // Save the old revision if needed.
  // No longer needed.
  // if (!$node->is_new && !empty($node->revision) && $node->vid) {
  // $node->old_vid = $node->vid;
  // }
  // *
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

    // XXX drupal_write_record('node', $node);
    // Call our modified function after putting the nid back into the
    // object.
    $node->nid = $defined_nid;
    data_export_import_drupal_write_record_via_insert_with_defined_id('node', $node);

    db_query('UPDATE {node_revisions} SET nid = %d WHERE vid = %d', $node->nid, $node->vid);
    $op = 'insert';
  }

  // No longer needed.
  // else {
  // drupal_write_record('node', $node, 'nid');
  // if (!empty($node->revision)) {
  // _node_save_revision($node, $user->uid);
  // db_query('UPDATE {node} SET vid = %d WHERE nid = %d',
  // $node->vid, $node->nid);
  // }
  // else {
  // _node_save_revision($node, $user->uid, 'vid');
  // }
  // $op = 'update';
  // }
  // *
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

    // Special case -- skip serial types if we are updating.
    // No longer needed.
    // if ($info['type'] == 'serial' && count($update)) {
    // continue;
    // }
    // *
    // For inserts, populate defaults from Schema if not already provided.
    if (!isset($object->$field) && !count($update) && isset($info['default'])) {
      $object->$field = $info['default'];
    }

    // Track serial fields so we can helpfully populate them after the query.
    // No longer needed.
    // if ($info['type'] == 'serial') {
    // $serials[] = $field;
    // Ignore values for serials when inserting data. Unsupported.
    // unset($object->$field);
    // }
    // *
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
  // No longer needed.
  // else {
  // $query = '';
  // foreach ($fields as $id => $field) {
  // if ($query) {
  // $query .= ', ';
  // }
  // $query .= $field . ' = ' . $placeholders[$id];
  // }
  // *
  // foreach ($update as $key) {
  // $conditions[] = "$key = " .
  // db_type_placeholder($schema['fields'][$key]['type']);
  // $values[] = $object->$key;
  // }
  // *
  // $query = "UPDATE {" . $table . "} SET $query WHERE " .
  // implode(' AND ', $conditions);
  // $return = SAVED_UPDATED;
  // }
  // *
  // Execute the SQL.
  if (db_query($query, $values)) {

    $a = 1;
    // No longer needed.
    // if ($serials) {
    // Get last insert ids and fill them in.
    // foreach ($serials as $field) {
    // $object->$field = db_last_insert_id($table, $field);
    // }
    // }
  }
  else {
    $return = FALSE;
  }

  // If we began with an array, convert back so we don't surprise the caller.
  if ($array) {
    $object = (array) $object;
  }

  return $return;
}

