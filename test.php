<?php

  // This file is used to run various functions etc from the command line.

require_once 'includes/vocabularies.inc';


// HERE - NEED TO SORT OUT WHAT HAPPENS IF A TERM HAS TWO PARENTS - THIS PRODUCES TWO TERM RECORDS 
// WITH THE SAME tid 

// THE FOLLOWING EXPORT/IMPORT TRIES TO UPDATE BOTH OF THESE RECORDS.

// NEED TO IDENTIFY RECORDS BY vid,tid AND PARENTS.

//data_export_import_export_vocabularies();

data_export_import_import_vocabularies ("sites/default/files/test_vocabularies");

//reset_taxonomy_vocabulary_hierarchy(10);

return;


/*
  $vocabularies_array2 = taxonomy_get_vocabularies();


  // NEEDS TO BE MADE TO WORK!
  foreach ($vocabularies_array2 as $vocabulary2) {


    echo "NEEDS TO WORK";

    $vocabulary_id2 = $vocabulary2->vid;
    reset_taxonomy_vocabulary_hierarchy($vocabulary_id2);


  }

*/

///session_destroy();

//reset_taxonomy_vocabulary_hierarchy('18');


return;



$tree = taxonomy_get_tree_with_reset(18, 0, -1, NULL, TRUE);

echo "here is the FIRST tree returned:\n";
var_export($tree);


taxonomy_del_term('1971');


$tree = taxonomy_get_tree_with_reset(18, 0, -1, NULL, TRUE);

echo "here is the SECOND tree returned:\n";
var_export($tree);




return;



$loaded_term = taxonomy_get_term('1958');

var_export($loaded_term);


$term = array(
	      'tid' => 1958, // If we add key 'tid' to this array then the function will update this term.
	      'vid' => 18, // Voacabulary ID
	      'name' => 'Drupalqwerty', // Term Name
	      'description' => 'descriptin goes here initi',
	      'synonyms' => "Drupletqweqwe\nsecond synonym init", // (Optional) Synonym of this term
	      'weight' => 0,
	      'parent' => array(0), // (Optional) Term ID of a parent term 
	      'relations' => array(0), // (Optional) Related Term IDs
	      );


taxonomy_save_term($term);


$loaded_term_after = taxonomy_get_term('1958');

var_export($loaded_term_after);
