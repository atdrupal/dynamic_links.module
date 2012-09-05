<?php
// $Id$

function dynamic_links_node_save(&$node) {
  dynamic_links_node_save_internal_links($node);
  
  if (TRUE) { // @TODO: If self URL changed
    dynamic_links_node_save_container_nodes($node);
  }
}

function dynamic_links_node_save_container_nodes($node) {
}

function dynamic_links_node_save_internal_links($node) {
  $urls = array();
  
  // Find the links
  foreach (_dynamic_links_fields_to_find_interal_urls($node->type) as $field) {
    if (isset($node->{$field})) {
      dynamic_links_node_find_internal_urls($node->{$field}, $urls);
    }
  }

  // Remove all existing internal-links
  db_query("DELETE FROM {dynamic_links_node} WHERE nid = %d", $node->nid);

  // Update internal links
  foreach (array_keys($urls) as $url) {
    if ($internal_path = _dynamic_links_valid_drupal_path($url)) {
      if ($content_nid = _dynamic_links_valid_content_id($internal_path)) {
        $sql  = "INSERT INTO {dynamic_links_node} (nid, content_nid)";
        $sql .= " VALUES (%d, %d)";

        db_query($sql, $node->nid, $content_nid);
      }
    }
  }
}

/**
 * @return Node ID if $internal_path is a node path, FALSE if it is not.
 */
function _dynamic_links_valid_content_id($interal_path) {
  if (strpos($interal_path, 'node/') === 0) {
    $interal_path = explode('/', $interal_path);

    if (isset($interal_path[1])) {
      $content_id = $interal_path[1];

      if (is_numeric($content_id)) {
        return $content_id;
      }
    }

    return FALSE;
  }

  $interal_path = drupal_get_normal_path($interal_path);
  if (strpos($interal_path, 'node/') === 0) {
    return _dynamic_links_valid_content_id($interal_path);
  }

  return FALSE;
}

/**
 * @return internal path if the $url is valid, FALSE if not.
 */
function _dynamic_links_valid_drupal_path($url) {
  global $base_path;
  $find['domain']       = "http://{$_SERVER['SERVER_NAME']}";
  $find['domain_www']   = str_replace('www.www.', 'www.', "http://www.{$_SERVER['SERVER_NAME']}");
  $find['s_domain']     = "https://{$_SERVER['SERVER_NAME']}";
  $find['s_domain_www'] = str_replace('www.www.', 'www.', "https://www.{$_SERVER['SERVER_NAME']}");

  if (strpos($url, 'http://') === 0) {
    // Absolute > Relative
    $find = array($base['domain'], $base['domain_www'], $base['s_domain'], $base['s_domain_www']);
    $url = str_replace($find, '', $url);
  }

  // Remove #...
  if (strpos($url, '#') !== FALSE) {
    $url = preg_replace('`#.+`', '', $url);
  }

  // Replace // to /
  $url = str_replace('//', '/', $url);

  if (strpos($url, $base_path) === 0) { // Only process drupal path
    return substr($url, strlen($base_path));
  }

  return FALSE;
}

function dynamic_links_node_find_internal_urls($string, &$urls) {
  if (is_array($string)) {
    foreach (array_keys($string) as $k) {
      _dynamic_links_find_links($string[$k]['value'], $urls);
    }

    return;
  }

  _dynamic_links_find_links($string, $urls);
}

/**
 * Field to find the node's internal links
 */
function _dynamic_links_fields_to_find_interal_urls($node_type) {
  // @TODO: It's possible that node type may not have body field.
  $fields = array('body', 'teaser');

  // Also check the node-type's fields
  foreach (content_fields(NULL, $node->type) as $field) {
    // Also check the cck text/textarea fields.
    if ($field['type'] === 'text') { // If it's text/textarea field
      if (!empty($field['text_processing'])) { // And it's not plain text field
        $fields[] = $field['field_name'];
      }
    }
  }
  
  return $fields;
}

/**
 * Find URLs in a string.
 *
 * @param $string String to find.
 */
function _dynamic_links_find_links($string, &$urls) {
  preg_match_all('`<a href="([^"]+)"`i', $string, $matches);

  foreach ($matches[1] as $url) {
    if (!isset($urls[$url])) {
      $urls[$url] = TRUE;
    }
  }
}
