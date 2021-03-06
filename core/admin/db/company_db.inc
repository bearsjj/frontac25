<?php
/**********************************************************************
    Copyright (C) FrontAccounting, LLC.
	Released under the terms of the GNU General Public License, GPL, 
	as published by the Free Software Foundation, either version 3 
	of the License, or (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
    See the License here <http://www.gnu.org/licenses/gpl-3.0.html>.
***********************************************************************/
include_once($path_to_root. "/admin/db/payment_terms_db.inc");

/*
	Update main or gl company setup.
*/
function update_company_prefs($params)
{
	begin_transaction(__FUNCTION__, func_get_args());

	$sql = "UPDATE ".TB_PREF."sys_prefs SET value = ";
	foreach($params as $name => $value) {
		if (!db_query($sql. db_escape($value). " WHERE name=".db_escape($name),
			 "The company prefferences could not be updated "))
			return false;
		// update cached value
		$_SESSION['SysPrefs']->prefs[$name] = $value;
	}

	commit_transaction();
	return true;
}
/*
	Get company preferences. Returns cached values from global variable SysPrefs
	or retrieved from database if SysPrefs values are not set.
	$prefs can be preference name, array of names, or null for all preferences.
	
*/
function get_company_pref($prefs = null)
{
	global $SysPrefs, $db_version;

	if (!isset($SysPrefs->prefs))	 // just after first login or reset
		$SysPrefs->refresh();

	$all = $SysPrefs->prefs;

	if ($prefs && is_string($prefs))
		return @$all[$prefs];

	if (!is_array($all))
		$all = array();

	return $all;
}

function get_company_prefs()
{
	return get_company_pref(null);
}

function set_company_pref($pref, $category, $type, $length, $value)
{
	begin_transaction(__FUNCTION__, func_get_args());

	$sql = "REPLACE `".TB_PREF."sys_prefs` SET `name`=".db_escape($pref).", `category`=".db_escape($category)
		.", `type`=".db_escape($type).", `length`=".db_escape($length).", `value`=".db_escape($value);
	$result = db_query($sql, "cannot set company pref");

	commit_transaction();
	return $result;

}

function get_base_sales_type()
{
	return get_company_pref('base_sales');
}

function get_company_extensions($id = -1) {
	global $path_to_root;

	$file = $path_to_root.($id == -1 ? '' : '/company/'.(int)$id).'/installed_extensions.php';
	$installed_extensions = array();
	if (is_file($file)) {
		include($file);
	}
	return $installed_extensions;
}

//---------------------------------------------------------------------------------------------
//
// Resets $theme references in users records to 'default'.
//
function clean_user_themes($theme)
{
	global $db_connections, $db;

	begin_transaction(__FUNCTION__, func_get_args());

	$comp = user_company();

	$connections = $db_connections;	// do not use db_connections directly here, or script will hang due to db_connections usage inside the loop
	foreach ($connections as $n => $conn) {
		$db = $_SESSION["wa_current_user"]->set_db_connection($n);
		$sql = "UPDATE {$conn['tbpref']}users SET theme='default' WHERE theme='$theme'";
		if (!db_query($sql, 'Cannot update user theme settings'))
			return false;
	}
	$db = $_SESSION["wa_current_user"]->set_db_connection($comp);
	$_SESSION['wa_current_user']->prefs->theme = 'default';

	commit_transaction();
	return true;
}
