<?php
/******************************************************************************/
/****************************** DELETION FUNCTIONS ****************************/
/******************************************************************************/
function lab_migration_delete_solution($solution_id)
{
	global $user;
	$root_path = lab_migration_path();
	$status = TRUE;
	//$solution_q = db_query("SELECT * FROM {lab_migration_solution} WHERE id = %d", $solution_id);
	$query = db_select('lab_migration_solution');
	$query->fields('lab_migration_solution');
	$query->condition('id', $solution_id);
	$solution_q = $query->execute();
	$solution_data = $solution_q->fetchObject();
	if (!$solution_data)
	{
		drupal_set_message(t('Invalid solution.'), 'error');
		return FALSE;
	} //!$solution_data
	//$experiment_q = db_query("SELECT * FROM {lab_migration_experiment} WHERE id = %d", $solution_data->experiment_id);
	$query = db_select('lab_migration_experiment');
	$query->fields('lab_migration_experiment');
	$query->condition('id', $solution_data->experiment_id);
	$experiment_q = $query->execute();
	$experiment_data = $experiment_q->fetchObject();
	if (!$experiment_data)
	{
		drupal_set_message(t('Invalid experiment.'), 'error');
		return FALSE;
	} //!$experiment_data
	/* deleting solution files */
	//$solution_files_q = db_query("SELECT * FROM {lab_migration_solution_files} WHERE solution_id = %d", $solution_id);
	$query = db_select('lab_migration_solution_files');
	$query->fields('lab_migration_solution_files');
	$query->condition('solution_id', $solution_id);
	$solution_files_q = $query->execute();
	while ($solution_files_data = $solution_files_q->fetchObject())
	{
		if (!file_exists($root_path . $solution_files_data->filepath))
		{
			$status = FALSE;
			drupal_set_message(t('Error deleting !file. File does not exists.', array(
				'!file' => $solution_files_data->filepath
			)), 'error');
			continue;
		} //!file_exists($root_path . $solution_files_data->filepath)
		/* removing solution file */
		if (!unlink($root_path . $solution_files_data->filepath))
		{
			$status = FALSE;
			drupal_set_message(t('Error deleting !file', array(
				'!file' => $solution_files_data->filepath
			)), 'error');
			/* sending email to admins */
			$params['standard']['subject'] = "[ERROR] Error deleting example file";
			$params['standard']['body'] = "Error deleting solution files by " . $user->uid . " at " . $_SERVER['HTTP_HOST'] . $_SERVER['REQUEST_URI'] . " :
        solution id : " . $solution_id . "
        file id : " . $solution_files_data->id . "
        file path : " . $solution_files_data->filepath;

		$email_to = $user_data->mail;
		$from = variable_get('lab_migration_from_email', '');
		$bcc = variable_get('lab_migration_bcc_email', '');
		$cc = variable_get('lab_migration_from_email', '');
		$params['standard']['headers'] = array(
			'From' => $from,
			'MIME-Version' => '1.0',
			'Content-Type' => 'text/plain; charset=UTF-8; format=flowed; delsp=yes',
			'Content-Transfer-Encoding' => '8Bit',
			'X-Mailer' => 'Drupal',
			'Cc' => $cc,
			'Bcc' => $bcc
		);
		if (!drupal_mail('lab_migration', 'standard', $email_to, language_default(), $params, $from, TRUE))
			drupal_set_message('Error sending email message.', 'error');

		} //!unlink($root_path . $solution_files_data->filepath)
		else
		{
			/* deleting example files database entries */
			db_delete('lab_migration_solution_files')->condition('id', $solution_files_data->id)->execute();
		}
	} //$solution_files_data = $solution_files_q->fetchObject()
	if (!$status)
		return FALSE;
	/* removing code folder */
	$ex_path = $experiment_data->proposal_id . '/EXP' . $experiment_data->number . '/CODE' . $solution_data->code_number;
	$dir_path = $root_path . $ex_path;
	if (is_dir($dir_path))
	{
		if (!rmdir($dir_path))
		{
			drupal_set_message(t('Error deleting folder !folder', array(
				'!folder' => $dir_path
			)), 'error');
			/* sending email to admins */
			$params['standard']['subject'] = "[ERROR] Error deleting folder";
			$params['standard']['body'] = "Error deleting folder " . $dir_path . " by " . $user->uid . " at " . $_SERVER['HTTP_HOST'] . $_SERVER['REQUEST_URI'];

		$email_to = $user_data->mail;
		$from = variable_get('lab_migration_from_email', '');
		$bcc = variable_get('lab_migration_bcc_email', '');
		$cc = variable_get('lab_migration_from_email', '');
		$params['standard']['headers'] = array(
			'From' => $from,
			'MIME-Version' => '1.0',
			'Content-Type' => 'text/plain; charset=UTF-8; format=flowed; delsp=yes',
			'Content-Transfer-Encoding' => '8Bit',
			'X-Mailer' => 'Drupal',
			'Cc' => $cc,
			'Bcc' => $bcc
		);
		if (!drupal_mail('lab_migration', 'standard', $email_to, language_default(), $params, $from, TRUE))
			drupal_set_message('Error sending email message.', 'error');

			return FALSE;
		} //!rmdir($dir_path)
	} //is_dir($dir_path)
	else
	{
		drupal_set_message(t('Cannot delete solution folder. !folder does not exists.', array(
			'!folder' => $dir_path
		)), 'error');
		return FALSE;
	}
	/* deleting solution dependency and solution database entries */
	db_delete('lab_migration_solution_dependency')->condition('solution_id', $solution_id)->execute();
	db_delete('lab_migration_solution')->condition('id', $solution_id)->execute();
	return $status;
}
function lab_migration_delete_experiment($experiment_id)
{
	$status = TRUE;
	$root_path = lab_migration_path();
	//$experiment_q = db_query("SELECT * FROM {lab_migration_experiment} WHERE id = %d", $experiment_id);
	$query = db_select('lab_migration_experiment');
	$query->fields('lab_migration_experiment');
	$query->condition('id', $experiment_id);
	$experiment_q = $query->execute();
	$experiment_data = $experiment_q->fetchObject();
	if (!$experiment_data)
	{
		drupal_set_message('Invalid experiment.', 'error');
		return FALSE;
	} //!$experiment_data
	/* deleting solutions */
	//$solution_q = db_query("SELECT * FROM {lab_migration_solution} WHERE experiment_id = %d", $experiment_id);  
	$query = db_select('lab_migration_solution');
	$query->fields('lab_migration_solution');
	$query->condition('experiment_id', $experiment_id);
	$solution_q = $query->execute();
	$delete_exp_folder = FALSE;
	while ($solution_data = $solution_q->fetchObject())
	{
		$delete_exp_folder = TRUE;
		if (!lab_migration_delete_solution($solution_data->id))
			$status = FALSE;
	} //$solution_data = $solution_q->fetchObject()
	if (!$delete_exp_folder)
	{
		return TRUE;
	} //!$delete_exp_folder
	if ($status)
	{
		$dir_path = $root_path . $experiment_data->proposal_id . '/EXP' . $experiment_data->number;
		if (is_dir($dir_path))
		{
			$res = rmdir($dir_path);
			if (!$res)
			{
				drupal_set_message(t('Error deleting experiment folder !folder', array(
					'!folder' => $dir_path
				)), 'error');
				/* sending email to admins */
				$params['standard']['subject'] = "[ERROR] Error deleting experiment folder";
				$params['standard']['body'] = "Error deleting folder " . $dir_path;

				$email_to = $user_data->mail;
				$from = variable_get('lab_migration_from_email', '');
				$bcc = variable_get('lab_migration_bcc_email', '');
				$cc = variable_get('lab_migration_from_email', '');
				$params['standard']['headers'] = array(
					'From' => $from,
					'MIME-Version' => '1.0',
					'Content-Type' => 'text/plain; charset=UTF-8; format=flowed; delsp=yes',
					'Content-Transfer-Encoding' => '8Bit',
					'X-Mailer' => 'Drupal',
					'Cc' => $cc,
					'Bcc' => $bcc
				);
				if (!drupal_mail('lab_migration', 'standard', $email_to, language_default(), $params, $from, TRUE))
					drupal_set_message('Error sending email message.', 'error');

				return FALSE;
			} //!$res
			else
			{
				return TRUE;
			}
		} //is_dir($dir_path)
		else
		{
			drupal_set_message(t('Cannot delete experiment folder. !folder does not exists.', array(
				'!folder' => $dir_path
			)), 'error');
			return FALSE;
		}
	} //$status
	return FALSE;
}
function lab_migration_delete_lab($lab_id)
{
	$status = TRUE;
	$root_path = lab_migration_path();
	//$proposal_q = db_query("SELECT * FROM {lab_migration_proposal} WHERE id = %d", $lab_id);
	$query = db_select('lab_migration_proposal');
	$query->fields('lab_migration_proposal');
	$query->condition('id', $lab_id);
	$proposal_q = $query->execute();
	$proposal_data = $proposal_q->fetchObject();
	if (!$proposal_data)
	{
		drupal_set_message('Invalid Lab.', 'error');
		return FALSE;
	} //!$proposal_data
	/* delete experiments */
	//$experiment_q = db_query("SELECT * FROM {lab_migration_experiment} WHERE proposal_id = %d", $proposal_data->id);
	$query = db_select('lab_migration_experiment');
	$query->fields('lab_migration_experiment');
	$query->condition('proposal_id', $proposal_data->id);
	$experiment_q = $query->execute();
	while ($experiment_data = $experiment_q->fetchObject())
	{
		if (!lab_migration_delete_experiment($experiment_data->id))
		{
			$status = FALSE;
		} //!lab_migration_delete_experiment($experiment_data->id)
	} //$experiment_data = $experiment_q->fetchObject()
	return $status;
}
/*function lab_migration_del_lab_pdf($lab_id)
{
	$root_path = lab_migration_path();
	$dir_path = $root_path . "latex/";
	$pdf_filename = "lab_" . $lab_id . ".pdf";
	if (file_exists($dir_path . $pdf_filename))
		unlink($dir_path . $pdf_filename);
}*/
function lab_migration_delete_dependency($dependency_id)
{
	global $user;
	$root_path = lab_migration_path();
	$status = TRUE;
	//$dependency_files_q = db_query("SELECT * FROM {lab_migration_dependency_files} WHERE id = %d", $dependency_id);
	$query = db_select('lab_migration_dependency_files');
	$query->fields('lab_migration_dependency_files');
	$query->condition('id', $dependency_id);
	$dependency_files_q = $query->execute();
	$dependency_files_data = $dependency_files_q->fetchObject();
	if (!$dependency_files_data)
	{
		drupal_set_message(t('Invalid dependency.'), 'error');
		return FALSE;
	} //!$dependency_files_data
	if (!file_exists($root_path . $dependency_files_data->filepath))
	{
		drupal_set_message(t('Error deleting !file. File does not exists.', array(
			'!file' => $dependency_files_data->filepath
		)), 'error');
		return FALSE;
	} //!file_exists($root_path . $dependency_files_data->filepath)
	/* removing dependency file */
	if (!unlink($root_path . $dependency_files_data->filepath))
	{
		$status = FALSE;
		drupal_set_message(t('Error deleting !file', array(
			'!file' => $dependency_files_data->filepath
		)), 'error');
		/* sending email to admins */
		$params['standard']['subject'] = "[ERROR] Error deleting dependency file";
		$params['standard']['body'] = "Error deleting dependency files by " . $user->uid . " at " . $_SERVER['HTTP_HOST'] . $_SERVER['REQUEST_URI'] . " :
			dependency id : " . $dependency_id . "
			file id : " . $dependency_files_data->id . "
			file path : " . $dependency_files_data->filepath;
		$email_to = $user_data->mail;
		$from = variable_get('lab_migration_from_email', '');
		$bcc = variable_get('lab_migration_bcc_email', '');
		$cc = variable_get('lab_migration_from_email', '');
		$params['standard']['headers'] = array(
			'From' => $from,
			'MIME-Version' => '1.0',
			'Content-Type' => 'text/plain; charset=UTF-8; format=flowed; delsp=yes',
			'Content-Transfer-Encoding' => '8Bit',
			'X-Mailer' => 'Drupal',
			'Cc' => $cc,
			'Bcc' => $bcc
		);
		if (!drupal_mail('lab_migration', 'standard', $email_to, language_default(), $params, $from, TRUE))
			drupal_set_message('Error sending email message.', 'error');
	} //!unlink($root_path . $dependency_files_data->filepath)
	else
	{
		/* deleting dependency files database entries */
		db_delete('lab_migration_dependency_files')->condition('id', $dependency_id)->execute();
	}
	return $status;
}
