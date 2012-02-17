<?php
/**
 * @file
 * Enables users to be exported and imported.
 *
 */

/**
 * Function to add items to the main menu.
 */
function data_export_import_users_add_items_to_menu(&$items) {

  $items['admin/content/data_export_import/users'] = array(
          'title' => 'Users',
          'page callback' => 'data_export_import_callback_export_users',
          'access arguments' => array('access data export import'),
          'weight' => 1,
          'description' => 'Export Users',
          'type' => MENU_LOCAL_TASK,
  );

  $items['admin/content/data_export_import/users/export'] = array(
          'title' => 'Export',
          'page callback' => 'data_export_import_callback_export_users',
          'access arguments' => array('access data export import'),
          'weight' => 2,
          'description' => 'Export Users',
          'type' => MENU_DEFAULT_LOCAL_TASK,
  );

  $items['admin/content/data_export_import/users/import'] = array(
          'title' => 'Import',
          'page callback' => 'data_export_import_callback_import_users',
          'access arguments' => array('access data export import'),
          'weight' => 3,
          'description' => 'Import Users',
          'type' => MENU_LOCAL_TASK,
  );
}

/**
 * Callback function to export users.
 */
function data_export_import_callback_export_users() {

  return drupal_get_form('data_export_import_export_users_form');
}

/**
 * Callback function to import users.
 */
function data_export_import_callback_import_users() {

  return drupal_get_form('data_export_import_import_users_form');
}

/**
 * Function to create form to export users.
 */
function data_export_import_export_users_form($form_state) {

  $form = array();

  // This stops the values being posted from being flattened into a
  // single level array.  It might be easier to extract the
  // information from a non-flattened array.
  $form['#tree'] = TRUE;

  ////  data_export_import_users_add_fieldset_to_export_form($form);

  $form['users'] = array(
          '#type' => 'fieldset',
          '#title' => t('Users'),
          '#collapsible' => TRUE,
          '#collapsed' => FALSE, );

  $form['users']['users'] = array(
          '#type' => 'checkbox',
          '#title' => 'Export all Users', );

  $form['submit'] = array(
          '#type' => 'submit',
          '#value' => t('Create dataset file'),
  );

  return $form;
}

/**
 * Function to create form to import users.
 */
function data_export_import_import_users_form($form_state) {

  $form = array();

  $form['users'] = array(
          '#type' => 'fieldset',
          '#title' => t('Users'),
          '#collapsible' => TRUE,
          '#collapsed' => FALSE,
  );

  // Get the contents of the dataset directory and create a list of
  // links to dataset files.
  $directory = file_directory_path() . "/data_export_import/users";
  $mask = '.dataset';
  $files = file_scan_directory($directory, $mask);

  // Sort them by the filename which is used as the key.  Since the
  // files are named using datetime stamps they will be listed in
  // date/time order.
  ksort($files);

  $options = array();
  $options['none'] = "None";
  foreach ($files as $file) {

    $options[$file->basename] = check_plain($file->basename);
  }

  $form['users']['users'] = array(
          '#type' => 'radios',
          '#title' => t('Please select file to import'),
          '#default_value' => 'none',
          '#options' => $options,
  );

  $form['submit'] = array(
          '#type' => 'submit',
          '#value' => 'Import dataset files',
  );

  return $form;
}

/**
 * Add fieldset to the data export form.
 *
 */
/*
function data_export_import_users_add_fieldset_to_export_form(&$form) {

  $form['users'] = array(
    '#type' => 'fieldset',
    '#title' => t('Users'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE, );

  $form['users']['users'] = array(
    '#type' => 'checkbox',
    '#title' => 'Export all Users', );
}
*/

/**
 * Function to process form to export users.
 */
function data_export_import_export_users_form_submit($form, &$form_state) {

  // This is the function which will carry out the exporting of the
  //users to a dataset file.
  data_export_import_export_users_to_file($form_state, $dataset_files_created);

  drupal_set_message(t("The following dataset files were created:"));

  foreach ($dataset_files_created as $dataset_file_name) {
    drupal_set_message(check_plain($dataset_file_name));
  }

  return TRUE;
}

/**
 * Function to process form to import users.
 */
function data_export_import_import_users_form_submit($form, &$form_state) {

  if ($form_state['values']['users'] != 'none') {

    $result = data_export_import_import_users($form_state['clicked_button']['#post']['users']);
    drupal_set_message(t('The Users dataset file %dataset_file was imported.', array('%dataset_file' => $form_state['clicked_button']['#post']['users'])));
  }
}

/**
 * Export the required dataset file.
 *
 */
function data_export_import_export_users_to_file($form_state, &$dataset_files_created) {

  // Check that the check box has been ticked.
  if ($form_state['clicked_button']['#post']['users']['users'] != 1) {

    // $dataset_files_created[] = data_export_import_export_users();
    drupal_set_message(t("Please tick the box to confirm the data export."));
    return TRUE;
  }

  // This will be the main array which will hold the data which will
  // be output to the dataset file.
  $dataset = array();

  // Here will add the list of roles to the dataset - this is so we can check
  // that the receiving instance has a matching set of roles.
  $dataset['roles'] = user_roles();

  $result = db_query("SELECT uid FROM {users}");
  while ($user = db_fetch_object($result)) {

    // We will not put the anonymous or admin users into the file.  These are
    // expected to exist as standard.
    if ($user->uid > 1) {

      // Attach the data for a user to the array.
      $dataset['users'][$user->uid] = user_load($user->uid);
    }
  }

  // Here we will serialize the array to convert it to a string which
  // can then be output to a file.
  $dataset_serialized = serialize($dataset);

  // Create the default directory to hold the datasets.
  $dataset_directory_parent_directory = file_directory_path() . "/data_export_import";
  file_check_directory($dataset_directory_parent_directory, $mode = FILE_CREATE_DIRECTORY);

  $dataset_directory = file_directory_path() . "/data_export_import/users/";
  file_check_directory($dataset_directory, $mode = FILE_CREATE_DIRECTORY);

  // Save the string as a file. This is the dataset data file.
  $file_name = format_date(time(), 'custom', 'Ymd_His') . "_users.dataset";
  $file_path_and_name = $dataset_directory . "/" . $file_name;

  file_save_data($dataset_serialized, $file_path_and_name, FILE_EXISTS_REPLACE);

  $dataset_files_created[] = $file_name;

  return TRUE;
}

/**
 * Make users list match the users from a dataset file.
 *
 * This function is to effectively 'import'  all the users in the dataset file.
 *
 * We will check that the roles on the receiving instance match the roles from
 * the dataset.
 *
 * Then we will bring in the users from the dataset file.
 *
 * NB - When this import has finished the users in the receiving
 * instance should be an exact match with the users in the imported
 * dataset file.  Think in terms of rsync with the '--delete'
 * option. This means that as well as importing new users and updating
 * existing users we need to delete users from the receiving instance
 * which are not in the imported dataset.
 *
 * This synchronisation will be carried out by two passes.
 *
 * First we will loop through the users in the receiving instance  and
 * check against the the imported dataset. If the user exists in the
 * dataset then it will be updated in the receiving instance.  If it
 * doesn't exist in the dataset then it will be deleted from the
 * receiving Drupal instance.
 *
 * The second pass will be a loop through the dataset - any users
 * which are in the dataset but are not in the receiving Drupal
 * instance will be created.
 *
 * This will effectively mean that the users have been sychronised
 * completely.
 *
 * NB - This function will create new users with the same ID as they had
 * originally when they were exported to the dataset file.
 *
 * @param string $file
 *   The dataset file which is being imported.
 *
 * @return bool
 *   TRUE if the import process ran without errors.
 */
function data_export_import_import_users($file) {

  // Load the dataset file into a variable.
  $file_content = file_get_contents(file_directory_path() . "/data_export_import/users/" . $file);

  // Decode the serialized data and store it in an array of objects.
  $file_content_as_array = unserialize($file_content);

  // Here we will pick up if the file contains no users.
  if (empty($file_content_as_array['users'])) {
    drupal_set_message("There are no users in the dataset file", 'error');
    return;
  }

  // This will check that the roles in the dataset file all
  // exist in the receiving instance. NB - The receiving instance may have extra
  // roles which have been created.  This is OK because even though there is not
  // an exact match at least the roles which are needed to enable the users to
  // be imported all exist.
  $current_roles = user_roles();

  // Loop through the roles from the dataset and check there is a corresponding
  // role in the receiving instance.
  foreach ($file_content_as_array['roles'] as $dataset_role_key => $dataset_role_value) {

    if (!isset($current_roles[$dataset_role_key])) {

      drupal_set_message(t('The receiving instance does not have a role with ID of %id and name of %name.', array('%id' => $dataset_role_key, '%name' => $dataset_role_value)));
      return;
    }
    elseif ($current_roles[$dataset_role_key] != $dataset_role_value) {

      drupal_set_message(t('The dataset contains a role with ID of %id and name of %name but the current role with that ID has a name of %current_role_name.', array('%id' => $dataset_role_key, '%name' => $dataset_role_value, '%current_role_name' => $current_roles[$dataset_role_key])));
      return;
    }
  }

  // Here we will loop through the users in the receiving
  // instance.  For each user we will look into the dataset file being
  // imported.  If the user exists in the dataset file then we
  // will check to see if it matches and update it if it doesn't - if it does not
  // exist then we will delete it.
  $result = db_query("SELECT uid FROM {users}");
  while ($user = db_fetch_object($result)) {

    // Ignore the anonymous and default admin users.
    if ($user->uid == 0 || $user->uid == 1) {

      // Go to the start of the while loop
      continue;
    }

    // See if a user with this ID exists in the dataset file.
    $a = $user->uid;
    if (!isset($file_content_as_array['users'][$user->uid])) {

      // No user with this ID exists in the dataset file.  Therefore we will
      // delete the current user as we want the current user list to exactly
      // match the list in the dataset.  The current user could well be a test
      // user which was created during development.
      $a = 1;
      user_delete(array(), $user->uid);
    }
    else {

      $a = 1;

      // Else if it does see if it is the same and if not then update the current
      // user with the data from the dataset file.
      $current_user = user_load($user->uid);
      $user_from_dataset_file = $file_content_as_array['users'][$user->uid];
      if ($current_user != $user_from_dataset_file) {

        // Current user needs to be updated from the dataset file.
        user_save((object) array('uid' => $user_from_dataset_file->uid), (array) $user_from_dataset_file);

        // Here we will update the password to match what the was originally output.
        $sql_statement = "UPDATE {users} SET pass= '" . $user_from_dataset_file->pass . "' WHERE uid = " . $user_from_dataset_file->uid;
        update_sql($sql_statement);
        }
    }
  }

  // Here we will loop through all of the users in the dataset file and create
  // any users which are in the dataset file but not in the current users.
  foreach( $file_content_as_array['users'] as $dataset_file_user_value) {
    
    if (!user_load($dataset_file_user_value->uid)) {

        
        // HERE HERE HERE - password needs to be saved correctly.
        // As the password has been exported in encrypted format we will need to dave the password 
        // without it being re-encrypted.
        // $dataset_file_user_value->pass = $dataset_file_user_value->pass;
        
        user_save(null, (array) $dataset_file_user_value);
        
        // Here we will update the password to match what the was originally output.
        $sql_statement = "UPDATE {users} SET pass= '" . $dataset_file_user_value->pass . "' WHERE uid = " . $dataset_file_user_value->uid;
        update_sql($sql_statement);
    }
  } 
}


/**
 * This resets the value of the hierarchy field for the vocabulary.
 *
 * The value of the hierarchy field in the vocabulary is used to set
 * up how the admin screen works when editing terms. I.e. If the value
 * is set to '2' this means that there are terms with multiple parents
 * and consequently the drag and drop interface pn the admin page will
 * be disabled.
 *
 * As the terms are being programmatically updated by this module the
 * hierarchy value my not be correct for the terms and so needs to be
 * reset.
 *
 * @param int $vid
 *   Vocabulary ID of the vocabulary having it's hierarchy reset.
 *
 * @return bool
 *   TRUE if the function ran without error.
 */
/*
function data_export_import_reset_taxonomy_vocabulary_hierarchy($vid) {

  // Get the tree of terms we are going to examine.
  $tree = data_export_import_taxonomy_get_tree_with_reset($vid, 0, -1, NULL, TRUE);

  // Set the hierarchy to the lowest level and increase it if relevant
  // terms are found.
  $hierarchy = '0';
  foreach ($tree as $term) {

    // Check this term's parent count.
    if (count($term->parents) > 1) {

      $hierarchy = '2';

      // Break out the foreach loop as the highest hierarchy value has
      // been set.
      break;
    }
    // Look for a single parent - but do not set the value if there is
    // a single parent value of '0' which is normal for terms with no
    // parents.
    elseif (count($term->parents) == 1 && '0' !== array_shift($term->parents)) {
      $hierarchy = '1';
    }
  }

  // Second parameter is set to TRUE to reset the cache.
  $vocabulary_object = taxonomy_vocabulary_load($vid, TRUE);

  // If the hieracrchy value has changed then save the new value back
  // to the database.
  if ($hierarchy != $vocabulary_object->hierarchy) {

    $vocabulary_object->hierarchy = $hierarchy;

    // Needs to be cast to an array as that is what the
    // taxonomy_save_vocabulary function expects.
    $vocabulary_array = (array) $vocabulary_object;
    taxonomy_save_vocabulary($vocabulary_array);
  }
}
*/

/*
   * *
   * This function overrides the taxonomy_get_tree() API function.
   *
   * The extra functionality is to provide a reset flag as a
   * parameter. This enables the static attributes to be unset. This is
   * important as this function is used multiple times during the course
   * of this module. If it was not possible to unset the statics the
   * values would stay the same between the function calls resulting in
   * stale data being returned.
   *
   * @param int $vid
   *   Which vocabulary to generate the tree for.
   *
   * @param int $parent
   *   The term ID under which to generate the tree. If 0, generate the
   *   tree for the entire vocabulary.
   *
   * @param int $depth
   *   Internal use only.
   *
   * @param int $max_depth
   *   The number of levels of the tree to return. Leave NULL to return
   *   all levels.
   *
   * @param bool $reset
   *   Whether or not to clear the static variables to force this
   *   function to get fresh data
   *
   * @return array
   *   An array of all term objects in the tree. Each term object is
   *   extended to  have "depth" and "parents" attributes in addition to
   *   its normal ones. Results are statically cached.
*/

/*
function data_export_import_taxonomy_get_tree_with_reset($vid, $parent = 0, $depth = -1, $max_depth = NULL, $reset = FALSE) {
  static $children, $parents, $terms;

  if ($reset) {
    unset($children[$vid]);
    unset($parents[$vid]);
    unset($terms[$vid]);
  }

  $depth++;

  // We cache trees, so it's not CPU-intensive to call get_tree() on a
  // term and its children, too.
  if (!isset($children[$vid])) {
    $children[$vid] = array();

    $result = db_query(db_rewrite_sql('SELECT t.tid, t.*, parent FROM {term_data} t INNER JOIN {term_hierarchy} h ON t.tid = h.tid WHERE t.vid = %d ORDER BY weight, name', 't', 'tid'), $vid);
    while ($term = db_fetch_object($result)) {
      $children[$vid][$term->parent][] = $term->tid;
      $parents[$vid][$term->tid][] = $term->parent;
      $terms[$vid][$term->tid] = $term;
    }
  }

  $max_depth = (is_null($max_depth)) ? count($children[$vid]) : $max_depth;
  $tree = array();
  if ($max_depth > $depth && !empty($children[$vid][$parent])) {
    foreach ($children[$vid][$parent] as $child) {
      $term = drupal_clone($terms[$vid][$child]);
      $term->depth = $depth;

      // The "parent" attribute is not useful, as it would show one
      // parent only.
      unset($term->parent);
      $term->parents = $parents[$vid][$child];
      $tree[] = $term;
      if (!empty($children[$vid][$child])) {
        $tree = array_merge($tree, data_export_import_taxonomy_get_tree_with_reset($vid, $child, $depth, $max_depth));
      }
    }
  }
  return $tree;
}
*/

/**
 * This function is a modified version of the taxonomy_save_term function.
 *
 * @param array &$form_values
 *   Array of term data which is to be saved.
 *
 * @return int
 *   $status value showing if the term was saved OK.
 */

/*
function data_export_import_taxonomy_save_term_with_defined_tid(&$form_values) {

  // I think this has the effect of making sure these values exist on
  // the array but not overwrite any existing values.
  $form_values += array(
    'description' => '',
    'weight' => 0,
  );

  // Insert the record into the term_data table.
  // We're using a direct insertion as the drupal_write_record
  // function does not allow us to insert a new row with a defined id
  // value.
  $query_result = db_query("INSERT INTO {term_data} (`tid`, `vid`, `name`, `description`, `weight`) VALUES (%d, %d, '%s', '%s', %d)", $form_values['tid'], $form_values['vid'], $form_values['name'], $form_values['description'], $form_values['weight']);
  if ($query_result == FALSE) {
    drupal_set_message(t("Insertion into term_data table failed."));
  }

  // Set up parameters used later when calling hook_taxonomy().
  $hook = 'insert';
  $status = SAVED_NEW;

  // Deal with the data in the related tables.
  db_query('DELETE FROM {term_relation} WHERE tid1 = %d OR tid2 = %d', $form_values['tid'], $form_values['tid']);
  if (!empty($form_values['relations'])) {
    foreach ($form_values['relations'] as $related_id) {
      if ($related_id != 0) {
        db_query('INSERT INTO {term_relation} (tid1, tid2) VALUES (%d, %d)', $form_values['tid'], $related_id);
      }
    }
  }

  db_query('DELETE FROM {term_hierarchy} WHERE tid = %d', $form_values['tid']);
  if (!isset($form_values['parent']) || empty($form_values['parent'])) {
    $form_values['parent'] = array(0);
  }
  if (is_array($form_values['parent'])) {
    foreach ($form_values['parent'] as $parent) {
      if (is_array($parent)) {
        foreach ($parent as $tid) {
          db_query('INSERT INTO {term_hierarchy} (tid, parent) VALUES (%d, %d)', $form_values['tid'], $tid);
        }
      }
      else {
        db_query('INSERT INTO {term_hierarchy} (tid, parent) VALUES (%d, %d)', $form_values['tid'], $parent);
      }
    }
  }
  else {
    db_query('INSERT INTO {term_hierarchy} (tid, parent) VALUES (%d, %d)', $form_values['tid'], $form_values['parent']);
  }

  db_query('DELETE FROM {term_synonym} WHERE tid = %d', $form_values['tid']);
  if (!empty($form_values['synonyms'])) {
    foreach (explode("\n", str_replace("\r", '', $form_values['synonyms'])) as $synonym) {
      if ($synonym) {
        // Currently changed from chop() to rtrim() as recommended by
        // coder_tough_love.
        db_query("INSERT INTO {term_synonym} (tid, name) VALUES (%d, '%s')", $form_values['tid'], rtrim($synonym));
      }
    }
  }

  // We're OK to run any hooks which are called. If we used
  // drupal_write_record() to insert the record then it would have
  // filled in the tid field with the newly created tid and passed it
  // back out via 'pass by reference'. Since we've already got the
  // tid field filled in as we're creating a new record but with the
  // defined (i.e. specified tid) then then field in $form_values is
  // already filled in.
  if (isset($hook)) {
    module_invoke_all('taxonomy', $hook, 'term', $form_values);
  }

  cache_clear_all();

  return $status;
}
   *
*/