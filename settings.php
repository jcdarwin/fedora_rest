<?php
// $Id$

/**
 * @file
 * Fedora_rest configuration file.
 *
 * IMPORTANT: a number of settings are specified through the web admin interface
 * when importing objects from Fedora, and may override those specified below.
 *
 */

// Authentication details for the Fedora server; if no authentication, then
// these should be set to NULL.
$fedora_user = 'fedoraAdmin';
$fedora_password = 'welcome';

// Drupal Site specific variables; these are required
$site_url = 'http://localhost';
$site_directory= '/var/www/html';

// Default values; these can be overridden by command line args
// specify an ID for the set of objects being imported
$any_collection = 'default:anyCollection';
