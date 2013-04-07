<?php

/**
 * Field handler which shows the link for translating translation task items.
 *
 * @ingroup views_field_handlers
 */
class tmgmt_local_task_handler_field_item_operations extends views_handler_field_entity {

  function render($values) {
    /**
     * @var TMGMTLocalTaskItem $item
     */
    $item = $this->get_value($values);

    $element = array();
    $element['#theme'] = 'links';
    $element['#attributes'] = array('class' => array('inline'));
    if (entity_access('translate', 'tmgmt_local_task_item', $item)) {
      $element['#links']['translate'] = array(
        'href' => 'translate/' . $item->tltid . '/item/' . $item->tltiid,
        'options' => array(
          'query' => array('destination' => current_path()),
          'title' => t('Translate'),
        ),
        'title' => t('translate'),
      );
    }
    return drupal_render($element);
  }

}