<?php
// $Id$

/**
 * @file
 *
 * Administration page callbacks for the fedora_rest module
 */

/**
 * Form builder. Configure fedora_rest module.
 *
 * @ingroup forms
 * @see system_settings_form().
 */
function fedora_rest_admin_config($form, &$form_state) {

  // create arrays for selecting options
  $users = array();
  $result = db_query("SELECT name FROM {users} WHERE status > :status ORDER BY name", array(':status' => 0));
  foreach ($result as $u) {
    $users[$u->name] = $u->name;
  }
  $vocabs = array();
  if (module_exists('taxonomy')) {
    foreach (taxonomy_get_vocabularies( ) as $v) {
      $vocabs[$v->name] = $v->name;
    }
  }

  // IMPORT OPTIONS: for importing batches of digital objects
  $form['fedora_rest_import'] = array(
    '#type' => 'fieldset',
    '#title' => t('Import options'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#weight' => -10,
    '#description' => t('Import settings from the last import are saved. You can also set import options here without importing to use them with the CLI import program.'),
  );
  $form['fedora_rest_import']['fedora_rest_import_flag'] = array(
    '#type' => 'checkbox',
    '#title' => t('Harvest metadata from Fedora Repository'),
    '#default_value' => FALSE,
    '#description' => t('Select this box to synchronize with your repository. Make sure you have correctly set all the parameters before clicking the "Save Configuration" button to start the import. NB: The import may take 15-30 seconds per 100 objects imported; for large collections you may want to use the CLI import program.'),
    '#weight' => -10,
  );

  // COLLECTION PARAMETERS: apply to a batch and stored in collection record
  $form['fedora_rest_import']['fedora_rest_batch'] = array(
    '#type' => 'fieldset',
    '#title' => t('Collection settings'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#description' => t('These parameters describe how a batch of digital objects is imported into Drupal.'),
    '#weight' => -5,
  );
  $form['fedora_rest_import']['fedora_rest_batch']['fedora_rest_collection_arg'] = array(
    '#type' => 'textfield',
    '#title' => t('Collection URI'),
    '#required' => TRUE,
    '#default_value' => variable_get( 'fedora_rest_collection_arg',
                                             FEDORA_REST_DEFAULT_COLLECTION ),
    '#maxlength' => 255,
    '#description' => t('Enter a Uniform Resource Id for the set of objects being imported; this can be the PID of the collection in the repository or an identifier that you assign for the part of a collection that is imported with these collection settings.'),
    '#weight' => -10,
  );
  $form['fedora_rest_import']['fedora_rest_batch']['fedora_rest_server_arg'] = array(
    '#type' => 'textfield',
    '#title' => t('Repository URL'),
    '#required' => TRUE,
    '#default_value' => variable_get('fedora_rest_server_arg',
                                         'http://localhost:8080/fedora'),
    '#maxlength' => 255,
    '#description' => t('Enter the URL of the Fedora repository REST API.'),
    '#weight' => -9,
  );
  $form['fedora_rest_import']['fedora_rest_batch']['fedora_rest_version_arg'] = array(
    '#type' => 'select',
    '#options' => drupal_map_assoc(array('3.0', '3.1', '3.2', '3.3', '3.4')),
    '#multiple' => FALSE,
    '#title' => t('REST API version'),
    '#default_value' => variable_get('fedora_rest_version_arg', '3.4'),
    '#description' => t('Select the Fedora repository version; this handles differences in the REST API that were introduced with version 3.2'),
    '#weight' => -6,
  );
  $form['fedora_rest_import']['fedora_rest_batch']['fedora_rest_predicate_arg'] = array(
    '#type' => 'textfield',
    '#title' => t('SPO query predicate'),
    '#required' => TRUE,
    '#default_value' => variable_get('fedora_rest_predicate_arg',
                                         FEDORA_REST_DEMO_PREDICATE),
    '#maxlength' => 255,
    '#description' => t('Enter the URI of the Predicate used in the Resource Index to relate digital objects you want to import to the Object that you enter in the next field; these fields will be used to create an SPO query of the form "* &lt;Predicate&gt; &lt;Object&gt;" to select digital objects.'),
    '#weight' => -5,
  );
  $form['fedora_rest_import']['fedora_rest_batch']['fedora_rest_object_arg'] = array(
    '#type' => 'textfield',
    '#title' => t('SPO query object'),
    '#required' => TRUE,
    '#default_value' => variable_get('fedora_rest_object_arg',
                                         FEDORA_REST_DEMO_COLLECTION),
    '#maxlength' => 255,
    '#description' => t('Enter the URI of, or String representing, the Object used in the Resource Index that digital objects you want to import are related to by the Predicate that you enter in the previous field; these fields will be used to create an SPO query of the form "* &lt;Predicate&gt; &lt;Object&gt;" to select digital objects.'),
    '#weight' => -4,
  );
  $form['fedora_rest_import']['fedora_rest_batch']['fedora_rest_nt_arg'] = array(
    '#type' => 'select',
    '#title' => t('Drupal node type'),
    '#options' => node_type_get_names(),
    '#multiple' => FALSE,
    '#default_value' => variable_get('fedora_rest_nt_arg', 'fedora_object'),
    '#description' => t('Enter the Drupal node type that imported objects will be assigned to.'),
    '#weight' => -3,
  );
  $form['fedora_rest_import']['fedora_rest_batch']['fedora_rest_owner_arg'] = array(
    '#type' => 'select',
    '#title' => t('Drupal node owner'),
    '#options' => $users,
    '#default_value' => variable_get('fedora_rest_owner_arg', 'admin'),
    '#description' => t('Select the user to mark as the author for each NEW node that is created (does not affect previously created nodes).'),
    '#weight' => -2,
  );
  if (module_exists('taxonomy') and ! empty($vocabs)) {
    $form['fedora_rest_import']['fedora_rest_batch']['fedora_rest_clear_vocab'] = array(
      '#type' => 'select',
      '#title' => t('Delete vocabulary terms'),
      '#options' => $vocabs,
      '#multiple' => TRUE,
      //'#size' => 10,
      '#default_value' => array(),
      '#description' => t('Select vocabularies to clear before importing. WARNING: Only clear vocabularies that will be rebuilt by the import!'),
      '#weight' => -1,
    );
  }
  $form['fedora_rest_import']['fedora_rest_batch']['fedora_rest_delete_arg'] = array(
    '#type' => 'checkbox',
    '#title' => t('Delete nodes before importing'),
    '#default_value' => FALSE,
    '#description' => t("Select this to delete all nodes that are in this set (ie, have been assigned the same Collection URI) before importing. Don't do this if you have anything that won't be regenerated by the import, like files or CCK fields, attached to the existing nodes."),
    '#weight' => 10,
  );

  // NODE PARAMETERS: apply to a Drupal node, used on node edit/create forms
//      (fedora_rest_node_fields() is defined in fedora_rest.module)
  $form['fedora_rest_import']['fedora_rest_node'] = fedora_rest_node_fields($vocabs);

  // DEVELOPMENT OPTIONS: for creating custom modules
  $form['fedora_rest_devel'] = array(
    '#type' => 'fieldset',
    '#title' => t('Development options'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' => t('These options are useful when you are developing custom modules based on the Fedora REST API.'),
    '#weight' => 0,
  );
  $form['fedora_rest_devel']['fedora_rest_limit_arg'] = array(
    '#type' => 'select',
    '#title' => t('Import limit'),
    '#options' => drupal_map_assoc(array('no limit', 1, 2, 4, 8, 16, 32, 64, 128)),
    '#multiple' => FALSE,
    '#required' => TRUE,
    '#default_value' => 'no limit',
    '#description' => t('Use this to limit the number of objects being imported in order to test the import process.'),
    '#weight' => 0,
  );
  if (module_exists('taxonomy')) {
    $form['fedora_rest_devel']['fedora_rest_dict_flag'] = array(
      '#type' => 'checkbox',
      '#title' => t('Generate the vocabulary dictionaries.'),
      '#default_value' => TRUE,
      '#description' => t('Enable this option to generate the vocabulary dictionary that maps names to vids for each vocabulary.'),
      '#weight' => 1,
    );
  }
  $form['fedora_rest_devel']['fedora_rest_trace_flag'] = array(
    '#type' => 'checkbox',
    '#title' => 'Turn on verbose tracing for debugging.',
    '#default_value' => variable_get('fedora_rest_trace_flag', FALSE),
    '#description' => t('This option enables the display of trace messages for each time a fedora_rest function is called (and for your module if you instrument it like fedora_rest.module).'),
    '#weight' => 2,
  );

  /**
  * Not sure whether we really need the following filter_form;
  * It's not supported in Drupal 7 so have commented it out here.
  */
  // $form['fedora_rest_import']['filter'] = filter_form( variable_get('fedora_rest_format', 0));

    $form['#submit'][] = 'fedora_rest_admin_config_submit';

    return system_settings_form($form);
}


/**
 * Form handler.
 *
 */
function fedora_rest_admin_config_submit($form, &$form_state) {
  /**
   * Note: form defined with fieldsets but $form['#tree'] was not set to TRUE
   * so the values here are flattened, not hierarchical under fieldsets
   * see Pro Drupal Development pg 234-235
   */

  /**
   * content settings
   */
  $dictflag = $form_state['values']['fedora_rest_dict_flag'];
  if ($dictflag) {
    $vdictionary = array();
    $fliptionary = array();
    foreach (taxonomy_get_vocabularies( ) as $vocab) {
      $vdictionary[$vocab->vid] = $vocab->name;
      $fliptionary[$vocab->name] = $vocab->vid;
    }
    variable_set('fedora_rest_vdictionary', $vdictionary);
    variable_set('fedora_rest_fliptionary', $fliptionary);
    drupal_set_message(t("Vocabulary dictionaries are in 'fedora_rest_vdictionary' (vid=>name) and 'fedora_rest_fliptionary' (name=>vid)"));
  }
  $vocabs = $form_state['values']['fedora_rest_clear_vocab'];
  if (is_array($vocabs)) {
    foreach ($vocabs as $key => $vname) {
      $vd = variable_get('fedora_rest_fliptionary', array());
      $vid = $vd[$vname];
      fedora_rest_delete_terms($vid);
      drupal_set_message(t("All terms in vocabulary $vname ($vid) have been deleted."));
    }
  }

  // $format = $form_state['values']['format'];
  $format = 0;
  variable_set('fedora_rest_format', $format);

  /**
   * import settings
   */
  $importflag = $form_state['values']['fedora_rest_import_flag'];
  if ($importflag) {
    // these options are accessed by hooks in fedora_rest.module
    variable_set('fedora_rest_xmlds_arg',
                      $form_state['values']['fedora_rest_xmlds_arg']);
        variable_set('fedora_rest_depth',
                     $form_state['values']['fedora_rest_depth']);
        variable_set('fedora_rest_tag_parents_flag',
                     $form_state['values']['fedora_rest_tag_parents_flag']);
        variable_set('fedora_rest_has_part_arg',
                     $form_state['values']['fedora_rest_has_part_arg']);
    if (module_exists('taxonomy')) {
      foreach (taxonomy_get_vocabularies( ) as $v) {
        $varg = 'fedora_rest_' . str_replace(' ', '_', $v->name) . '_arg';
        if (isset($form_state['values'][$varg])) {
          variable_set($varg, $form_state['values'][$varg]);
        }
      }
    }

    // these options are passed to the import_items method
    $collection = $form_state['values']['fedora_rest_collection_arg'];
    $server = $form_state['values']['fedora_rest_server_arg'];

		// load the default settings for authentication
		$fedora_rest_settings = "settings.php";
		require($fedora_rest_settings);
    $usr = $fedora_user;
    $password = $fedora_password;

    $version = $form_state['values']['fedora_rest_version_arg'];
    $predicate = $form_state['values']['fedora_rest_predicate_arg'];
    $object = $form_state['values']['fedora_rest_object_arg'];
    $ntype = $form_state['values']['fedora_rest_nt_arg'];
    $nuser = $form_state['values']['fedora_rest_owner_arg'];
    $urow_uid = db_query("SELECT uid FROM {users} WHERE name = :name", array(':name' => check_plain($nuser)))->fetchField();
    if (isset($urow_uid)) {
      $nuid = $urow_uid;
    }
    else {
      drupal_set_message(t("WARNING: user ':nuser' not found; nodes will be owned by 'admin'.", array(':nuser' => $nuser)), 'warning');
      $nuid = 1;
    }
    $delete = $form_state['values']['fedora_rest_delete_arg'];
    $limit = $form_state['values']['fedora_rest_limit_arg'];
    if ($limit == 'no limit') {
      $limit = 0;
    }

    $rtn = fedora_rest_import_items($server, $usr, $password, $collection, $predicate, $object,
                                $ntype, $nuid, $delete, $limit, $format, $version);
    if ($rtn === FALSE) {
      drupal_set_message(t('Import aborted!'), 'error');
    }
    // import_items collected messages, now have to re-set them for display
    fedora_rest_reset_messages( );
  }
}

// * <http://quakestudies.canterbury.ac.nz#hasType> <info:fedora/qsr:content_partner>
function fedora_rest_retrieve_spo($spo_subject, $spo_predicate, $spo_object ) {
	fedora_rest_trace("fedora_rest_retrieve_spo($spo_subject,$spo_predicate,$spo_object)");
	global $_fedora_rest_import_messages;

  $server = variable_get('fedora_rest_server_arg', 'http://localhost:8080/fedora');
  $version = variable_get('fedora_rest_version_arg', '3.4');

	// load the default settings for authentication
	$fedora_rest_settings = "settings.php";
	require($fedora_rest_settings);
	$usr = $fedora_user;
	$password = $fedora_password;

	$res_limit = variable_get('fedora_rest_limit_arg', 0);

	try {
			$client = new FedoraClient( $server, $usr, $password, $version );
	} catch (Exception $e) {
			$exception = '<pre>' . print_r($e, TRUE) . '</pre>';
			drupal_set_message(t("Exception Object:: $exception\n"), 'error');
			fedora_rest_save_messages();
			return FALSE;
	}

  /***********************************************************************
   * find objects with the specified relationship in the resource index
   */
  $spo_query = $spo_subject . ' ' . $spo_predicate . ' ' . $spo_object;

  fedora_rest_trace("SPO Query: $spo_query (view source to see tags)");

  try {
    $results = $client->findTriples( $spo_query, $res_limit );
  }
  catch (Exception $e) {
    $exception = '<pre>' . print_r($e, TRUE) . '</pre>';
    drupal_set_message("Exception Object:: $exception\n", 'error');
    fedora_rest_save_messages();
    return FALSE;
  }

  $count = 0;
  foreach ($results as $URI => $rdf) {
    $count++;
  }

  drupal_set_message(t("!cnt Fedora spo results retrieved", array('!cnt' => $count)),
                         'status' );
  fedora_rest_save_messages();

	return $results;
}

/**
 * Return an array of object dc from the repo.
 */
function fedora_rest_retrieve_objects_dc($pids) {
	fedora_rest_trace("fedora_rest_retrieve_objects_dc($pids)");
	global $_fedora_rest_import_messages;

  $server = variable_get('fedora_rest_server_arg', 'http://localhost:8080/fedora');
  $version = variable_get('fedora_rest_version_arg', '3.4');

	// load the default settings for authentication
	$fedora_rest_settings = "settings.php";
	require($fedora_rest_settings);
	$usr = $fedora_user;
	$password = $fedora_password;

	try {
			$client = new FedoraClient( $server, $usr, $password, $version );
	} catch (Exception $e) {
			$exception = '<pre>' . print_r($e, TRUE) . '</pre>';
			watchdog(__FUNCTION__, t("Exception Object:: $exception\n"), array(), WATCHDOG_ERROR);
			drupal_set_message(t("Exception Object:: $exception\n"), 'error');
			fedora_rest_save_messages();
			return FALSE;
	}
	
  /***********************************************************************
   * return an array of the fedora objects
   */
  $count = 0;
  $fos = array();
  foreach ($pids as $pid) {

		$q = "pid=$pid";
		$request = array(
			'title' => 'true',
			'type' => 'true',
			'query' => $q,
		);
		try {
			$response = $client->findObjects( $request );	
		}
		catch (Exception $e) {
			$exception = '<pre>' . print_r($e, TRUE) . '</pre>';
			drupal_set_message("Exception Object:: $exception\n", 'error');
			fedora_rest_save_messages();
			return FALSE;
		}
		$rcount = count($response);
		if ($rcount != 1) {
			drupal_set_message(t("Search for objects with !q returned !r!\n",
																 array('!q' => $q, '!r' => $rcount)),
															 'error');
			fedora_rest_save_messages();
			return FALSE;
		}

    if ($fo = $response[$pid]) {
      $fo->pid = $pid;
      $status_msg = "retrieving Fedora object $pid => ";
      drupal_set_message($status_msg, 'status');
      $count++;
      fedora_rest_save_messages();
      $fos[] = (object)$fo;
    }
    else {
      drupal_set_message(t("Unexpected response from repository for pid=$pid\n", array(!pid => $pid)), 'error');
      fedora_rest_save_messages();
      return FALSE;
     }
     
  }

  drupal_set_message(t("!cnt Fedora objects retrieved", array('!cnt' => $count)),
                         'status' );
  fedora_rest_save_messages();
  
  return $fos;
}

/**
 * Return an array of datastream objects from the repo.
 */
function fedora_rest_retrieve_objects_ds($pids, $dsId = NULL) {
	fedora_rest_trace("fedora_rest_retrieve_objects_ds($pids, $dsId)");
	global $_fedora_rest_import_messages;

  $server = variable_get('fedora_rest_server_arg', 'http://localhost:8080/fedora');
  $version = variable_get('fedora_rest_version_arg', '3.4');

	// load the default settings for authentication
	$fedora_rest_settings = "settings.php";
	require($fedora_rest_settings);
	$usr = $fedora_user;
	$password = $fedora_password;

	try {
			$client = new FedoraClient( $server, $usr, $password, $version );
	} catch (Exception $e) {
			$exception = '<pre>' . print_r($e, TRUE) . '</pre>';
			drupal_set_message(t("Exception Object:: $exception\n"), 'error');
			fedora_rest_save_messages();
			return FALSE;
	}
	
  /***********************************************************************
   * return an array of the fedora objects
   */
  $count = 0;
  $fos = array();
  foreach ($pids as $pid) {

		try {
			$response = $client->getDatastreamDissemination( $pid, $dsId );
		}
		catch (Exception $e) {
			$exception = '<pre>' . print_r($e, TRUE) . '</pre>';
			drupal_set_message("Exception Object:: $exception\n", 'error');
			fedora_rest_save_messages();
			return FALSE;
		}
		$rcount = count($response);
		if ($rcount != 1) {
			drupal_set_message(t("Search for objects for datastream !d returned !r!\n",
																 array('!d' => $dsId, '!r' => $rcount)),
															 'error');
			fedora_rest_save_messages();
			return FALSE;
		}
	
		foreach ($response as $fo) {
      $fo->pid = $pid;
      $status_msg = "retrieving Fedora object $pid => ";
      drupal_set_message($status_msg, 'status');
      $count++;
      fedora_rest_save_messages();
      $fos[] = (object)$fo;
    }
     
  }

  drupal_set_message(t("!cnt Fedora objects retrieved", array('!cnt' => $count)),
                         'status' );
  fedora_rest_save_messages();
  
  return $fos;
}

function fedora_rest_get_next_pid() {
	fedora_rest_trace("fedora_rest_get_next_pid()");
	global $_fedora_rest_import_messages;

  $server = variable_get('fedora_rest_server_arg', 'http://localhost:8080/fedora');
  $version = variable_get('fedora_rest_version_arg', '3.4');

	// load the default settings for authentication
	$fedora_rest_settings = "settings.php";
	require($fedora_rest_settings);
	$usr = $fedora_user;
	$password = $fedora_password;

	try {
			$client = new FedoraClient( $server, $usr, $password, $version );
	} catch (Exception $e) {
			$exception = '<pre>' . print_r($e, TRUE) . '</pre>';
			drupal_set_message(t("Exception Object:: $exception\n"), 'error');
			fedora_rest_save_messages();
			return FALSE;
	}

	try {
		$pid = $client->getNextPID();
	}
	catch (Exception $e) {
		$exception = '<pre>' . print_r($e, TRUE) . '</pre>';
		drupal_set_message("Exception Object:: $exception\n", 'error');
		fedora_rest_save_messages();
		return FALSE;
	}

  drupal_set_message(t("Next pid retrieved: !pid", array('!pid' => $pid)),
                         'status' );
  fedora_rest_save_messages();
  
  return $pid;
	
}

function fedora_rest_ingest($pid, $args, $object) {
	fedora_rest_trace("fedora_rest_ingest()");
	global $_fedora_rest_import_messages;

  $server = variable_get('fedora_rest_server_arg', 'http://localhost:8080/fedora');
  $version = variable_get('fedora_rest_version_arg', '3.4');

	// load the default settings for authentication
	$fedora_rest_settings = "settings.php";
	require($fedora_rest_settings);
	$usr = $fedora_user;
	$password = $fedora_password;

	try {
			$client = new FedoraClient( $server, $usr, $password, $version );
	} catch (Exception $e) {
			$exception = '<pre>' . print_r($e, TRUE) . '</pre>';
			drupal_set_message(t("Exception Object:: $exception\n"), 'error');
			fedora_rest_save_messages();
			return FALSE;
	}

	try {
		$resp = $client->ingest($pid, $args, $object);
	}
	catch (Exception $e) {
		$exception = '<pre>' . print_r($e, TRUE) . '</pre>';
		drupal_set_message("Exception Object:: $exception\n", 'error');
		fedora_rest_save_messages();
		return FALSE;
	}

  drupal_set_message(t("Ingest response: !resp", array('!resp' => $resp)),
                         'status' );
  fedora_rest_save_messages();
  
  return $resp;	
}

function fedora_rest_add_datastream($pid, $dsID, $args, $managedFile, $inlineXML) {
	fedora_rest_trace("fedora_rest_add_datastream()");
	global $_fedora_rest_import_messages;

  $server = variable_get('fedora_rest_server_arg', 'http://localhost:8080/fedora');
  $version = variable_get('fedora_rest_version_arg', '3.4');

	// load the default settings for authentication
	$fedora_rest_settings = "settings.php";
	require($fedora_rest_settings);
	$usr = $fedora_user;
	$password = $fedora_password;

	try {
			$client = new FedoraClient( $server, $usr, $password, $version );
	} catch (Exception $e) {
			$exception = '<pre>' . print_r($e, TRUE) . '</pre>';
			drupal_set_message(t("Exception Object:: $exception\n"), 'error');
			fedora_rest_save_messages();
			return FALSE;
	}

	try {
		$resp = $client->addDatastream($pid, $dsID, $args, $managedFile, $inlineXML);
	}
	catch (Exception $e) {
		$exception = '<pre>' . print_r($e, TRUE) . '</pre>';
		drupal_set_message("Exception Object:: $exception\n", 'error');
		fedora_rest_save_messages();
		return FALSE;
	}

  drupal_set_message(t("Add datastream response: !resp", array('!resp' => $resp)),
                         'status' );
  fedora_rest_save_messages();
  
  return $resp;	
}

function fedora_rest_get_datastream($pid, $dsID, $format='xml', $validateChecksum=NULL, $asOfDateTime=NULL) {
	fedora_rest_trace("fedora_rest_get_datastream()");
	global $_fedora_rest_import_messages;

  $server = variable_get('fedora_rest_server_arg', 'http://localhost:8080/fedora');
  $version = variable_get('fedora_rest_version_arg', '3.4');

	// load the default settings for authentication
	$fedora_rest_settings = "settings.php";
	require($fedora_rest_settings);
	$usr = $fedora_user;
	$password = $fedora_password;

	try {
			$client = new FedoraClient( $server, $usr, $password, $version );
	} catch (Exception $e) {
			$exception = '<pre>' . print_r($e, TRUE) . '</pre>';
			drupal_set_message(t("Exception Object:: $exception\n"), 'error');
			fedora_rest_save_messages();
			return FALSE;
	}

	try {
		$resp = $client->getDatastream($pid, $dsID, $format, $validateChecksum, $asOfDateTime);
	}
	catch (Exception $e) {
		$exception = '<pre>' . print_r($e, TRUE) . '</pre>';
		drupal_set_message("Exception Object:: $exception\n", 'error');
		fedora_rest_save_messages();
		return FALSE;
	}

  drupal_set_message(t("Get datastream response: !resp", array('!resp' => $resp)),
                         'status' );
  fedora_rest_save_messages();
  
  return $resp;	
}

function fedora_rest_get_datastream_dissemination($pid, $dsID, $binary=FALSE, $asOfDateTime=NULL) {
	fedora_rest_trace("fedora_rest_get_datastream_dissemination()");
	global $_fedora_rest_import_messages;

  $server = variable_get('fedora_rest_server_arg', 'http://localhost:8080/fedora');
  $version = variable_get('fedora_rest_version_arg', '3.4');

	// load the default settings for authentication
	$fedora_rest_settings = "settings.php";
	require($fedora_rest_settings);
	$usr = $fedora_user;
	$password = $fedora_password;

	try {
			$client = new FedoraClient( $server, $usr, $password, $version );
	} catch (Exception $e) {
			$exception = '<pre>' . print_r($e, TRUE) . '</pre>';
			drupal_set_message(t("Exception Object:: $exception\n"), 'error');
			fedora_rest_save_messages();
			return FALSE;
	}

	try {
		$resp = $client->getDatastreamDissemination($pid, $dsID, $binary, $asOfDateTime);
	}
	catch (Exception $e) {
		$exception = '<pre>' . print_r($e, TRUE) . '</pre>';
		drupal_set_message("Exception Object:: $exception\n", 'error');
		fedora_rest_save_messages();
		return FALSE;
	}

  drupal_set_message(t("Get datastream dissemination response: !resp", array('!resp' => $resp)),
                         'status' );
  fedora_rest_save_messages();
  
  return $resp;	
}

function fedora_rest_modify_datastream($pid, $dsID, $args, $modifiedFile, $curlWorkaround) {
	fedora_rest_trace("fedora_rest_modify_datastream()");
	global $_fedora_rest_import_messages;

  $server = variable_get('fedora_rest_server_arg', 'http://localhost:8080/fedora');
  $version = variable_get('fedora_rest_version_arg', '3.4');

	// load the default settings for authentication
	$fedora_rest_settings = "settings.php";
	require($fedora_rest_settings);
	$usr = $fedora_user;
	$password = $fedora_password;

	try {
			$client = new FedoraClient( $server, $usr, $password, $version );
	} catch (Exception $e) {
			$exception = '<pre>' . print_r($e, TRUE) . '</pre>';
			drupal_set_message(t("Exception Object:: $exception\n"), 'error');
			fedora_rest_save_messages();
			return FALSE;
	}

	try {
		$resp = $client->modifyDatastream($pid, $dsID, $args, $modifiedFile, $curlWorkaround);
	}
	catch (Exception $e) {
		$exception = '<pre>' . print_r($e, TRUE) . '</pre>';
		drupal_set_message("Exception Object:: $exception\n", 'error');
		fedora_rest_save_messages();
		return FALSE;
	}

  drupal_set_message(t("Modify datastream response: !resp", array('!resp' => $resp)),
                         'status' );
  fedora_rest_save_messages();
  
  return $resp;	
}


function fedora_rest_purge_object($pid, $args) {
	fedora_rest_trace("fedora_rest_purge_object()");
	global $_fedora_rest_import_messages;

  $server = variable_get('fedora_rest_server_arg', 'http://localhost:8080/fedora');
  $version = variable_get('fedora_rest_version_arg', '3.4');

	// load the default settings for authentication
	$fedora_rest_settings = "settings.php";
	require($fedora_rest_settings);
	$usr = $fedora_user;
	$password = $fedora_password;

	try {
			$client = new FedoraClient( $server, $usr, $password, $version );
	} catch (Exception $e) {
			$exception = '<pre>' . print_r($e, TRUE) . '</pre>';
			drupal_set_message(t("Exception Object:: $exception\n"), 'error');
			fedora_rest_save_messages();
			return FALSE;
	}

	try {
		$resp = $client->purgeObject($pid, $args);
	}
	catch (Exception $e) {
		$exception = '<pre>' . print_r($e, TRUE) . '</pre>';
		drupal_set_message("Exception Object:: $exception\n", 'error');
		fedora_rest_save_messages();
		return FALSE;
	}

  drupal_set_message(t("Purge object response: !resp", array('!resp' => $resp)),
                         'status' );
  fedora_rest_save_messages();
  
  return $resp;	
}

function fedora_rest_import_items(  $server, $usr, $password, $collection, $spo_predicate,
                                    $spo_object, $node_type, $node_UID=1,
                                    $delete=FALSE, $res_limit=0, $format=0,
                                    $version='3.4') {
    fedora_rest_trace("fedora_rest_import_items($server,$usr,$password,$collection,$spo_predicate,$spo_object,$node_type,$node_UID,$delete,$res_limit,$format,$version)");
    global $user;
    global $_fedora_rest_import_messages;

    $_fedora_rest_import_messages = array();
    try {
        $client = new FedoraClient( $server, $usr, $password, $version );
    } catch (Exception $e) {
        $exception = '<pre>' . print_r($e, TRUE) . '</pre>';
        drupal_set_message(t("Exception Object:: $exception\n"), 'error');
        fedora_rest_save_messages();
        return FALSE;
    }

  /***********************************************************************
   * Get or Create collection record in Drupal database
   */
  $coll_id = db_query("SELECT id FROM {fedora_rest_collection} WHERE uri = :uri AND server = :server AND version = :version", array(':uri' => check_plain($collection), ':server' => check_plain($server), ':version' => check_plain($version)))->fetchField();
  if ($coll_id) {
    $collection_ID = $coll_id;
  }
  else {
    $id = db_insert('fedora_rest_collection')
  ->fields(array(
    'uri' => check_plain($collection),
    'server' => check_plain($server),
    'version' => check_plain($version),
    'description' => '',
  ))
  ->execute();
    $collection_ID = $id;
  }


  /***********************************************************************
   * Clear existing nodes if requested, otherwise node ids will be preserved
   */
  if ($delete) {
    if ($user->uid == 1) {
      $count = fedora_rest_delete_nodes($collection_ID);
      drupal_set_message(t("!cnt drupal nodes from collection $collection_ID deleted prior to import", array('!cnt' => $count)), 'status');
    }
    else {
      drupal_set_message(t("WARNING: you must run import as admin user to delete nodes, delete option ignored."), 'warning');
    }
  }


  /***********************************************************************
   * find objects with the specified relationship in the resource index
   */
  $spo_query = '* <' . $spo_predicate . '> <' . $spo_object . '>';
  fedora_rest_trace("SPO Query: $spo_query (view source to see tags)");

  try {
    $results = $client->findTriples( $spo_query, $res_limit );
  }
  catch (Exception $e) {
    $exception = '<pre>' . print_r($e, TRUE) . '</pre>';
    drupal_set_message("Exception Object:: $exception\n", 'error');
    fedora_rest_save_messages();
    return FALSE;
  }

  if (strpos($spo_predicate, 'hasModel') === FALSE) {
    $content_model = FEDORA_REST_DEFAULT_MODEL;
  }
  else {
    $content_model = $spo_object;
  }

  /***********************************************************************
   * create a drupal node for each fedora object
   */
  $count = 0;
  foreach ($results as $pid_URI => $rdf) {
    $path = preg_split('|/|', $pid_URI);
    $pid = array_pop($path);

    $q = "pid=$pid";
    $request = array(
      'title' => 'true',
      'query' => $q,
    );
     try {
      $response = $client->findObjects( $request );
     }
    catch (Exception $e) {
      $exception = '<pre>' . print_r($e, TRUE) . '</pre>';
      drupal_set_message("Exception Object:: $exception\n", 'error');
      fedora_rest_save_messages();
      return FALSE;
    }
    $rcount = count($response);
    if ($rcount != 1) {
      drupal_set_message("Search for objects with !q returned !r!\n",
                                 array('!q' => $q, '!r' => $rcount),
                               'error');
      fedora_rest_save_messages();
      return FALSE;
    }

    if ($fo = $response[$pid]) {
      $fo->model = $content_model;
      $fo->type  = $node_type;
      $fo->persistent_id = $pid;
      $fo->collection_id = $collection_ID;
      $fo->uid = $node_UID;
      $fo->fmt = $format;
      $status_msg = "importing Fedora object $pid => ";
      $nid = fedora_rest_make_node($fo);
      $status_msg .= "Drupal node $nid";
      drupal_set_message($status_msg, 'status');
      $count++;
      fedora_rest_save_messages();
    }
    else {
      drupal_set_message(t("Unexpected response from repository for pid=$pid\n", array(!pid => $pid)), 'error');
      fedora_rest_save_messages();
      return FALSE;
     }
  }

  $recount = fedora_rest_reorder_nodes($collection_ID);

  drupal_set_message(t("!cnt drupal nodes imported", array('!cnt' => $count)),
                         'status' );
  fedora_rest_save_messages();
  return $count;
 }


/**
 * Create a Drupal node for the Fedora object
 */
function fedora_rest_make_node($fedora_object) {
  global $user;

  // make sure fedoraObject SXE elements are cast to their string values
  $node->type = (string) $fedora_object->type;
  $node->title = (string) $fedora_object->title;
  $node->body = '';
  $node->import_model = (string) $fedora_object->model;
  $node->persistent_id = (string) $fedora_object->persistent_id;
  $node->collection_id = (string) $fedora_object->collection_id;

  $node->created = REQUEST_TIME;
  $node->changed = $node->created;

  if ($fedora_object->fmt) {
    // apply a default Input Format
    $node->format = $fedora_object->fmt;
  }

  // use defaults configured in Drupal for this content type, rather than
  // these hard-coded values
  //$node->status = 0;
  //$node->promote = 0;
  //$node->sticky = 0;

  // set import options for hooks in fedora_rest.module
  $node->fedora_rest_xmlds_arg = variable_get('fedora_rest_xmlds_arg', '');
  $node->fedora_rest_depth = variable_get('fedora_rest_depth', 0);
  $node->fedora_rest_tag_parents_flag = variable_get('fedora_rest_tag_parents_flag', FALSE);
  $node->fedora_rest_has_part_arg = variable_get('fedora_rest_has_part_arg', 'rel:hasPart');
  if (module_exists('taxonomy')) {
    foreach (taxonomy_get_vocabularies( ) as $v) {
      $varg = 'fedora_rest_' . str_replace(' ', '_', $v->name) . '_arg';
      if (isset($form_state['values'][$varg])) {
        $node->{$varg} = variable_get($varg, '');
      }
    }
  }

  // check whether node exists, if so keep same nid but replace object record
  $fo_nid = db_query("SELECT nid FROM {fedora_rest_node} WHERE persistent_id = :persistent_id", array(':persistent_id' => $node->persistent_id))->fetchField();
  if (isset($fo_nid)) {
    $node->nid = $fo_nid;
    // need the vid also to make sure node_revisions is updated
    $no_vid = db_query("SELECT vid FROM {node} WHERE nid = :nid", array(':nid' => $fo_nid))->fetchField();
    $node->vid = $no_vid;
  }
  else {
    // set ownership for new node
    $node->uid = $fedora_object->uid;
  }

  node_save($node);

  return $node->nid;
}

/**
 * Delete all existing Drupal nodes that were created for this collection
 */
function fedora_rest_delete_nodes($collection_ID) {

  // delete any nodes from this collection
  $count = 0;
  $result = db_query("SELECT nid FROM {fedora_rest_node} WHERE collection_id = :collection_id", array(':collection_id' => $collection_ID));

  foreach ($result as $row) {
    node_delete($row->nid);
    $count++;
  }

  return $count;
}

/**
 * Modify the create date for each node so taxonomy_selet_nodes will
 * get them in ascending title order when "order by node.created DESC"
 */
function fedora_rest_reorder_nodes($collection_ID) {

  // get a base timestamp
  $createtime = REQUEST_TIME;
  // select nodes from this collection
  $count = 0;
  $result = db_query("SELECT n.nid FROM {node} n INNER JOIN {fedora_rest_node} frn ON n.nid=frn.nid WHERE collection_id = :collection_id ORDER BY n.title DESC", array(':collection_id' => $collection_ID));

  foreach ($result as $row) {
    db_update('node')
  ->fields(array(
    'created' => $createtime,
    'changed' => $createtime,
  ))
  ->condition('nid', $row->nid)
  ->execute();
    $createtime++;
    $count++;
  }

  return $count;
}

/**
 *
 *
 */
function fedora_rest_save_messages( ) {
  global $_fedora_rest_import_messages;

  $mess = drupal_get_messages( );
  if (isset($mess['error'])) {
    foreach ($mess['error'] as $message) {
      $_fedora_rest_import_messages[] = "ERROR: $message\n";
    }
  }
  if (isset($mess['warning'])) {
    foreach ($mess['warning'] as $message) {
      $_fedora_rest_import_messages[] = "WARNING: $message\n";
    }
  }
  if (isset($mess['status'])) {
    foreach ($mess['status'] as $message) {
      $_fedora_rest_import_messages[] = "$message\n";
    }
  }
  if (isset($mess['debug'])) {
    foreach ($mess['debug'] as $message) {
      $_fedora_rest_import_messages[] = "$message\n";
     }
  }
}

/**
 *
 *
 */
function fedora_rest_reset_messages( ) {
  global $_fedora_rest_import_messages;

  foreach ($_fedora_rest_import_messages as $message) {
    if (0 === strpos($message, 'ERROR: ')) {
      drupal_set_message(check_plain(substr($message, 7)), 'error');
    }
    elseif (0 === strpos($message, 'WARNING: ')) {
      drupal_set_message(check_plain(substr($message, 9)), 'warning');
    }
    else {
      drupal_set_message(check_plain($message), 'status');
     }
  }
 }

/**
 *
 * Return the name of the site-specific settings file.
 */ 
function _getSettingsFile() {
// get current location
	$module_directory = dirname(__FILE__);

// Collect site-specific variables
	$config = "$module_directory/settings.php";
	
	if (! file_exists($config)) die("Required config file not found: $config\n");
	return $config;
}
