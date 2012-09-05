<?php
// $Id$

/**
 * Batch starting form.
 */
function dynamic_links_update() {
  $form['submit'] = array(
    '#type'  => 'submit',
    '#value' => 'Start updating dynamic links for existing contents',
  );

  return $form;
}

function dynamic_links_update_submit($form, &$form_state) {
  $sql = "SELECT nid FROM {node} ORDER BY nid ASC";
  $query = db_query($sql);
  while ($v = db_fetch_object($query)) {
    $operations[] = array(
      '_dynamic_links_node_save', array($v->nid)
    );
  }

  $batch['operations'] = $operations;
  $batch['finished'] = '_dynamic_links_batch_finished';

  batch_set($batch);
}
