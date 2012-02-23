<?php
/**
 * @file
 *   This file defines the Data Export Import module drush commands.
 *
 *   To find out how to use the command run `drush help dei-export`
 *   to see the command options.
 */

/**
 * Implementation of hook_drush_command().
 *
 * This function specifies which commands the drush module has made
 * available. 
 *
 * @return
 *   An associative array describing your command(s).
 */
function dei_drush_command() {
  $items = array();

  // The 'dei-export' command
  $items['dei-export'] = array(
    'description' => "Exports Data Export Import dataset profiles.",
    'arguments' => array(
      'profile' => 'The type of dataset profile.',
    ),
    'options' => array(
      'options' => 'List of comma separated options to supply to the command.',
    ),
    'examples' => array(
      'drush dei-export terms' => 'Export taxonomy terms to a file.',
    ),
    'aliases' => array('dei-ex'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  // The 'dei-import' command
  $items['dei-import'] = array(
    'description' => "Imports Data Export Import dataset files.",
    'arguments' => array(
      'file' => 'The name of the file being imported.',
    ),
    'options' => array(
      'options' => 'List of comma separated options to supply to the command.',
    ),
    'examples' => array(
      'drush dei-import file options=20120223_170526_all_content_types_page' => 'Import a dataset from a file.',
    ),
    'aliases' => array('dei-im'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  // Commandfiles may also add topics.  These will appear in the list
  // of topics when `drush topic` is executed. To view this topic, run
  // `drush --include=/full/path/to/examples topic` NB - This still
  // needs the include directive - not sure how to run this to get the
  // topic included by default.  NB - This has currently be left in
  // place for reference.
  $items['dei-exposition'] = array(
    'description' => 'Ruminations on the true meaning and philosophy of sandwiches.',
    'hidden' => TRUE,
    'topic' => TRUE,
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
    'callback' => 'drush_print_file',
    'callback arguments' => array(dirname(__FILE__) . '/sandwich-topic.txt'),
  );

  return $items;
}

/**
 * Implementation of hook_drush_help().
 *
 * This function is called whenever a drush user calls
 * 'drush help '. This hook is optional. If a command
 * does not implement this hook, the command's description is used instead.
 *
 * This hook is also used to look up help metadata, such as help
 * category title and summary.  See the comments below for a description.
 *
 * @param
 *   A string with the help section (prepend with 'drush:')
 *
 * @return
 *   A string with the help text for your command.
 */
function dei_drush_help($section) {

  switch ($section) {

    case 'drush:dei-export':
 
      return dt("This command will export a dataset profile to a data file.");

    case 'drush:dei-import':
 
      return dt("This command will import a dataset file.");

    case 'meta:dei:title':

      // The 'title' meta item is used to name a group of
      // commands in `drush help`.  If a title is not defined,
      // the default is "All commands in ___", with the
      // specific name of the commandfile (e.g. sandwich).
      // Command files with less than four commands will 
      // be placed in the "Other commands" section, _unless_
      // they define a title.  It is therefore preferable
      // to not define a title unless the file defines a lot
      // of commands.
      return dt("Data Export Import commands");

    case 'meta:dei:summary':

      // The 'summary' meta item is displayed in `drush help --filter`,
      // and is used to give a general idea what the commands in this
      // command file do, and what they have in common.
      return dt("Drush commands to carry out functions of the Data Export Import module.");
  }
}

/**
 * Implementation of drush_hook_COMMAND_validate().
 *
 * The validate command should exit with
 * `return drush_set_error(...)` to stop execution of
 * the command.  In practice, calling drush_set_error
 * OR returning FALSE is sufficient.  See drush.api.php
 * for more details.
 */
function drush_dei_export_validate() {
  $name = posix_getpwuid(posix_geteuid());
  // if ($name['name'] !== 'root') {
  if ($name['name'] == 'root') {
    return drush_set_error('MAKE_IT_YOUSELF', dt('What? Make your own sandwich.'));
  }
}

/**
 * A drush command callback. This is where the action takes place.
 *
 * This function will handle exporting data profiles to data files -
 * it can be extended by the addition of further profiles.
 */
function drush_dei_export($profile) {

  // NB - Here we need to get the options from the profile files.
  switch ($profile) {

    case 'terms':

      $msg = data_export_import_export_taxonomy_terms();
      break;

    case 'users':

      $msg = data_export_import_export_users_to_file();
      break;

    case 'nodes':

      $msg = "nodes option was selected.";

      $list_of_content_types = drush_get_option('content-types');
      $array_of_content_types = explode(',', $list_of_content_types);

      data_export_import_all_content_types_export_to_file_using_drush($array_of_content_types);
      $msg = "drush msg";

      break;

    default:
      $msg = "No valid argument was supplied.";
  }

  drush_print($msg);
  return;

  $str_spreads = '';
  if ($spreads = drush_get_option('spreads')) {
    $list = implode(' and ', explode(',', $spreads));
    $str_spreads = ' with just a dash of ' . $list;
  }
  $msg = dt('Okay. xxxyyy Enjoy this !filling sandwich!str_spreads.',
         array('!filling' => $filling, '!str_spreads' => $str_spreads)
           );
  drush_print("\n" . $msg . "\n");
  drush_print(file_get_contents(dirname(__FILE__) . '/sandwich.txt'));
}

/**
 * A drush command callback. This is where the action takes place.
 *
 * This function will handle exporting data profiles to data files -
 * it can be extended by the addition of further profiles.
 */
function drush_dei_import($dataset_file) {

  // NB - Here we need to get the options from the profile files.
  switch ($dataset_file) {

    case 'file':

      $msg = "Here we will be importing a dataset file.";
      break;

    default:
      $msg = "No valid argument was supplied to the dei-import command.";
  }

  drush_print($msg);
  return;

}
