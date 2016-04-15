<?php

/**
 * @file
 * Contains \Drupal\data_export_import\Controller\DataExportImport.
 */

namespace Drupal\data_export_import\Controller;

use Drupal\Core\Controller\ControllerBase;
use Drupal\Core\Url;

/**
 * Controller for building the block instance add form.
 */
class DataExportImport extends ControllerBase {

  public function CallbackOverview() {
    
    $element = array(
      '#markup' => "Please click on one of the profile tabs to export or import data.",
    );
    
    return $element;
  }
  
  public function ExportNodes() {

    $out ="";
    $out .= "Hello world!";
    $element = array(
      '#markup' => $out,
    );
    return $element;
  }

  public function ImportNodes() {

    $out ="";
    $out .= "Hello world!";
    $element = array(
      '#markup' => $out,
    );
    return $element;
  }

  public function Exportterms() {

    $out ="";
    $out .= "Hello world!";
    $element = array(
      '#markup' => $out,
    );
    return $element;
  }

  public function ImportTerms() {

    $out ="";
    $out .= "Hello world!";
    $element = array(
      '#markup' => $out,
    );
    return $element;
  }

  public function ExportUser() {

    $out ="";
    $out .= "Hello world!";
    $element = array(
      '#markup' => $out,
    );
    return $element;
  }

  public function ImportUser() {

    $out ="";
    $out .= "Hello world!";
    $element = array(
      '#markup' => $out,
    );
    return $element;
  }

}

