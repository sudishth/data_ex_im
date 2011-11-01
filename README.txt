
CONTENTS OF THIS FILE
---------------------

 * Overview

OVERVIEW
--------

The purpose of this module is to provide a way to move data between
Drupal instances. 

This is not easy to achieve as Drupal stores configuration and data in
the same database. Also, Drupal can be extended and customised which
makes creating a standard data export/import module effectively
impossible. 

This module will provide a framework for exporting datasets as files
and also for importing these dataset files. That way if the module is
installed on two separate Drupal instances it will be possible to
export data as a dataset file from one instance and import it into a
different Drupal instance. 

This module will be customisable and extendable via plugins or a
similar method to take into account different Drupal builds. This
means that if a Drupal instance has a very specialised setup it will
be possible to create a new dataset profile to export/import data. 

The module should include some initial dataset profiles which can be
used on standard Drupal instances. These would enable the following
datasets to be exported/imported. 

 * Taxonomy vocabularies and terms
 * Basic nodes such as page, story.
 * Users

When exporting all data should be exported - when importing the data
should be used in a way to make sure that the receiving Drupal
instance ends up with matching data. Careful checks will be needed to
ensure ID's match, that the data remains consistent.

Effectively this module will be similar to the backup_migrate module
except that the backup_migrate profiles will be extended. Currently
the backup_migrate profiles only allow for the choice of which tables
will be exported/imported and a few other settings. This module will
use the Druapl API to read data and produce rich data files which will
contain all data needed to reproduce a particular dataset. It will
probably also use the Drupal API to recreate data. It may also be
possible to use SQL directly to export/import data.

The primary usage for this module is envisioned to be exporting
datasets from a live Drupal site and importing this live data into a
new updated version of the site which developers have been working
on. Once the import has completed then the newly developed site can
become the new live site. This has the advantage of being able to fall
back to the original live site if there is an unforseen problem
between the new site and the live site data.

It will be necessary to carry out proper checks when importing
datasets. E.g.. when node data is being imported it will be necessary
to check that the receiving site has the correct content type and
fields etc. It may be necessary to use the UUID module to provide
unique ID's.
