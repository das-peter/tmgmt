<?php

/*
 * @file
 * Entity classes for Translation Management entities.
 */

/**
 * Entity class for the tmgmt_translator entity.
 *
 * @ingroup translator
 */
class TMGMTTranslator extends Entity {

  /**
   * The ID of the translator.
   *
   * @var integer
   */
  public $tid;

  /**
   * Machine readable name of the translator.
   *
   * @var string
   */
  public $name;

  /**
   * Label of the translator.
   *
   * @var string
   */
  public $label;

  /**
   * Plugin name of the translator.
   *
   * @see TMGMTTranslator::getPluginController().
   *
   * @type string
   */
  public $plugin;

  /**
   * Translator type specific settings.
   *
   * @var array
   */
  public $settings;

  /**
   * Overrides Entity::__construct().
   */
  public function __construct(array $values = array()) {
    parent::__construct($values, 'tmgmt_translator');
  }

  /**
   * Returns the translator plugin controller of this translator.
   *
   * @return TMGMTTranslatorPluginControllerInterface
   */
  public function getTranslatorController() {
    return tmgmt_translator_plugin_controller($this->translator_plugin);
  }

}

/**
 * Entity class for the tmgmt_job entity.
 *
 * @ingroup job
 */
class TMGMTJobItem extends Entity {

  /**
   * The source plugin that provides the item.
   *
   * @see TMGMTMap::getPluginController().
   *
   * @var varchar
   */
  public $plugin;

  /**
   * The identifier of the translation job.
   *
   * @var int
   */
  public $tjid;

  /**
   * Type of this item, used by the plugin to identify it.
   *
   * @var string
   */
  public $item_type;

  /**
   * Id of the item.
   *
   * @var integer
   */
  public $item_id;

  /**
   * The translated data in the same structure.
   *
   * Only the parts that actually are translated are present in this array.
   *
   * @var array
   */
  public $translated_data;

  /**
   * Array of the data to be translated.
   *
   * The structure is similar to the form API in the way that it is a possibly
   * nested array with the following properties whose presence indicate that the
   * current element is a text that might need to be translated.
   *
   * - #text: The text to be translated.
   * - #label: The label that might be shown to the translator.
   * - #comment: (Optional) A comment with additional information.
   * - #translate: Either TRUE or FALSE. Only text that has this property set to
   *   TRUE must be translated.
   *
   * The key can be an alphanumeric string.
   *
   * @return
   *   A data array structured in the
   *
   * @see TMGMTJobItem::getFlattenedData()
   */
  public function getSourceData() {
    return $this->getSourceController()->getData($this);
  }

  /**
   * Returns the configured plugin controller.
   *
   * @return TMGMTSourcePluginControllerInterface
   */
  public function getSourceController() {
    return tmgmt_source_plugin_controller($this->source_plugin);
  }

  /**
   * Add translated data to a job item.
   *
   * Note: This does not merge existing data which already exists below the
   * defined key and will replace it.
   *
   * @param $translated_data
   *   Nested array of translated data. Can either be a single text entry, the
   *   whole data structure or parts of it.
   * @param $key
   *   (Optional) Either a flattened key (a 'key1:key2:key3' string) or a nested
   *   one, e.g. array('key1', 'key2', 'key2'). Defaults to an empty array which
   *   means that it will replace the whole translated data array.
   */
  public function addTranslatedData($translated_data, $key = array()) {
    if (empty($key)) {
      $key = array();
    }
    if (!is_array($key)) {
      $key = explode(':', $key);
    }
    if (!is_array($this->translated_data)) {
      $this->translated_data = array();
    }
    drupal_array_set_nested_value($this->translated_data, $key, $translated_data);
    $this->save();
  }

}

/**
 * Entity class for the tmgmt_job entity.
 *
 * @ingroup job
 */
class TMGMTJob extends Entity {

  /**
   * A new translation job, can not yet be submitted.
   */
  const STATE_NEW = 0;

  /**
   * A prepared translation job, not yet submitted to a translator.
   */
  const STATE_PREPARED = 1;

  /**
   * A translation job that has been submitted to the translator.
   */
  const STATE_SUBMITTED = 2;

  /**
   * A translation job that has been rejected by the translator.
   *
   * The response contains information why the job was rejected.
   */
  const STATE_REJECTED = 3;

  /**
   * The translation job was translated and needs to be reviewed.
   */
  const STATE_REVIEW = 4;

  /**
   * The translation has been accepted and the job is finished.
   */
  const STATE_ACCEPTED = 5;

  /**
   * The translation job has been cancelled.
   */
  const STATE_CANCELLED = 6;

  /**
   * Translation job identifier.
   *
   * @var integer
   */
  public $tjid;

  /**
   * Current state of the translation job
   * @var type
   */
  public $state;

  /**
   * Language to be translated from.
   *
   * @var string
   */
  public $source_language;

  /**
   * Language into which the data needs to be translated.
   *
   * @var varchar
   */
  public $target_language;

  /**
   * Reference to the used translator of this job.
   *
   * @see TMGMTJob::getTranslator().
   *
   * @var string
   */
  public $translator;

  /**
   * Translator specific configuration and context information for this job.
   *
   * @var array
   */
  public $translator_context;

  /**
   * Remote identification of this job.
   *
   * @var integer
   */
  public $translator_id;

  /**
   * Overrides Entity::__construct().
   */
  public function __construct(array $values = array()) {
    parent::__construct($values, 'tmgmt_job');
  }

  /**
   * Static method to retrieve a labeled list of all available states.
   *
   * @return array
   *   A list of all available states.
   */
  static function getStatesList() {
    return array(
      self::STATE_NEW => t('New'),
      self::STATE_PREPARED => t('Prepared'),
      self::STATE_SUBMITTED => t('Submitted'),
      self::STATE_REJECTED => t('Rejected'),
      self::STATE_REVIEW => t('Needs review'),
      self::STATE_ACCEPTED => t('Accepted'),
      self::STATE_CANCELLED => t('Cancelled'),
    );
  }

  /**
   * Returns if the state of this transaction is new.
   *
   * @return
   *   TRUE if the state is new, FALSE otherwise.
   */
  public function isNew() {
    return $this->getState() == TMGMTJob::STATE_NEW;
  }

  /**
   * Returns if the state of this transaction is prepared.
   *
   * @return
   *   TRUE if the state is prepared, FALSE otherwise.
   */
  public function isPrepared() {
    return $this->getState() == TMGMTJob::STATE_PREPARED;
  }

  /**
   * Returns if the state of this transaction is submitted.
   *
   * @return
   *   TRUE if the state is submitted, FALSE otherwise.
   */
  public function isSubmitted() {
    return $this->getState() == TMGMTJob::STATE_SUBMITTED;
  }

  /**
   * Returns if the state of this transaction is rejected.
   *
   * @return
   *   TRUE if the state is rejected, FALSE otherwise.
   */
  public function isRejected() {
    return $this->getState() == TMGMTJob::STATE_REJECTED;
  }

  /**
   * Returns if the state of this transaction is needs review.
   *
   * @return
   *   TRUE if the state is needs review, FALSE otherwise.
   */
  public function isNeedsReview() {
    return $this->getState() == TMGMTJob::STATE_REVIEW;
  }

  /**
   * Returns if the state of this transaction is accepted.
   *
   * @return
   *   TRUE if the state is accepted, FALSE otherwise.
   */
  public function isAccepted() {
    return $this->getState() == TMGMTJob::STATE_ACCEPTED;
  }

  /**
   * Returns if the state of this transaction is cancelled.
   *
   * @return
   *   TRUE if the state is cancelled, FALSE otherwise.
   */
  public function isCancelled() {
    return $this->getState() == TMGMTJob::STATE_CANCELLED;
  }

  /**
   * Propagates the returned job item translations to the sources.
   */
  public function propagateTranslatedData() {
    if ($this->state == self::STATE_ACCEPTED) {
      foreach ($this->getItems() as $item) {
        $item->getSourceController()->saveTranslation($item);
      }
    }
  }

  /**
   * @param TMGMTTranslatorResponseInterface $response
   */
  public function addResponse(TMGMTTranslatorResponseInterface $response) {
    if ((($message = $response->toString()) !== NULL)) {
      $this->response[REQUEST_TIME][] = $response->toString();
    }
  }

  /**
   * Updates the state of a job to the state supplied by the response object.
   *
   * @param TMGMTTranslatorResponseInterface $response
   *   The response object.
   *
   * @return bool
   *   TRUE if the state of the job entity has been updated, FALSE otherwise.
   */
  public function setStateFromResponse(TMGMTTranslatorResponseInterface $response) {
    $return = FALSE;
    if (($state = $response->toJobState()) !== NULL) {
      $return = $this->setState($state);
    }
    if ((($message = $response->toString()) !== NULL)) {
      $this->addMessage($response->toString());
    }
    return $return;
  }

  /**
   * Adds the item to the translation job.
   *
   * @param $item
   *    The translation job item.
   *
   * @return TMGMTJobItem
   *   The updated item
   */
  public function addItem(TMGMTJobItem $item) {
    // Save the job if it hasn't yet been saved.
    if (empty($this->tjid)) {
      $this->save();
    }
    $item->tjid = $this->tjid;
    $item->save();
  }

  /**
   * Returns the job items.
   *
   * @return
   *   Array of translation job items.
   */
  public function getItems() {
    $query = new EntityFieldQuery();
    $query->entityCondition('entity_type', 'tmgmt_job_item');
    $query->propertyCondition('tjid', $this->tjid);
    $results = $query->execute();
    if (!empty($results['tmgmt_job_item'])) {
      return tmgmt_job_item_load_multiple(array_keys($results['tmgmt_job_item']));
    }
    return array();
  }

  /**
   * Returns the translator plugin for this job.
   *
   * @return TMGMTTranslatorPluginControllerInterface
   *   The translator plugin controller or FALSE.
   */
  public function getTranslatorPlugin() {
    if ($this->translator) {
      return tmgmt_translator_plugin_controller($this->translator);
    }
    return FALSE;
  }

  /**
   * Returns the state of the job. Can be one of the job state constants.
   *
   * @return
   *   The state of the job.
   */
  public function getState() {
    if (isset($this->state)) {
      return $this->state;
    }
    return NULL;
  }

  /**
   * Add a log message for this job.
   *
   * @param $message
   *   The message to store in the log. Keep $message translatable by not
   *   concatenating dynamic values into it! Variables in the message should be
   *   added by using placeholder strings alongside the variables argument to
   *   declare the value of the placeholders. See t() for documentation on how
   *   $message and $variables interact.
   *
   * @param $arguments
   *   (Optional) Array of variables to replace in the message on display.
   *
   */
  public function addMessage($message, $arguments = array()) {
    $message = entity_create('tmgmt_job_message', array(
      'tmjid' => $this->tjid,
      'state_before' => isset($this->old_state) ? $this->old_state : $this->getState(),
      'state_after' => $this->getState(),
      'message' => $message,
      'arguments' => $arguments,
    ));
    $message->save();
  }

  /**
   * Set the state of the job to needs review.
   *
   * @see TMGMTJob::addLogMessage()
   */
  public function needsReview($message, $arguments = array()) {
    $this->setState(TMGMTJob::STATE_REVIEW);
    $this->addMessage($message);
  }

  /**
   * Set the state of the job to prepared.
   *
   * @see TMGMTJob::addLogMessage()
   */
  public function prepared($message = NULL, $arguments = array()) {
    $this->setState(TMGMTJob::STATE_PREPARED);
    $this->addMessage($message);
  }

  /**
   * Set the state of the job to submitted.
   *
   * @see TMGMTJob::addLogMessage()
   */
  public function submitted($message = NULL, $arguments = array()) {
    $this->setState(TMGMTJob::STATE_SUBMITTED);
    $this->addMessage($message);
  }

  /**
   * Set the state of the job to accepted
   *
   * @see TMGMTJob::addLogMessage()
   */
  public function accepted($message = NULL, $arguments = array()) {
    $this->setState(TMGMTJob::STATE_REVIEW);
    $this->addMessage($message);
    $this->propagateTranslatedData();
  }

  /**
   * Set the state of the job to finished.
   *
   * @see TMGMTJob::addLogMessage()
   */
  public function finished($message = NULL, $arguments = array()) {
    // @todo: To be defined.
  }

  /**
   * Set the state of the job to rejected.
   *
   * @see TMGMTJob::addLogMessage()
   */
  public function rejected($message = NULL, $arguments = array()) {
    $this->setState(TMGMTJob::STATE_REJECTED);
    $this->addMessage($message);
  }

  /**
   * Set the state of the job to cancelled.
   *
   * Note: Use TMGMTJob::cancelTranslation() to cancel a translation job if
   * possible.
   *
   * @see TMGMTJob::addLogMessage()
   */
  protected function cancelled($message = NULL, $arguments = array()) {
    $this->setState(TMGMTJob::STATE_CANCELLED);
    $this->addMessage($message);
  }

  /**
   * Updates the state of the job.
   *
   * @param $state
   *   The new state of the job. Has to be one of the state constants defined
   *   in the state
   */
  public function setState($state) {
    // Return TRUE if the state could be set. Return FALSE otherwise.
    switch ($state) {
      case self::STATE_ACCEPTED:
      case self::STATE_NEW:
      case self::STATE_PREPARED:
      case self::STATE_REJECTED:
      case self::STATE_REVIEW:
      case self::STATE_SUBMITTED:
      case self::STATE_CANCELLED;
        $this->old_state = $this->getState();
        $this->state = $state;
        return TRUE;
    }
    return FALSE;
  }

  /**
   * Checks whether a job is translatable by checking if it is prepared and
   * the translator can handle it.
   *
   * @return bool
   *   TRUE if the job is prepared and can be translated, FALSE otherwise.
   */
  public function isTranslatable() {
    if ($this->isPrepared() && $plugin = $this->getTranslatorPlugin()) {
      if ($plugin->canTranslate($this)) {
        return TRUE;
      }
    }
    return FALSE;
  }

  /**
   * Request the translation of a job from the translator.
   *
   * @return
   *   The updated job status.
   */
  public function requestTranslation() {
    $translator = $this->getTranslatorPlugin();
    $response = $translator->requestTranslation($this);
    $this->setStateFromResponse($response);
    $this->save();
    return $this->getState();
  }

  /**
   * Attempts to cancel the translation job.
   *
   * Already accepted jobs can not be cancelled, submitted jobs only if
   * supported by the translator plugin.
   *
   * @return
   *   TRUE if the translation job was cancelled, FALSE otherwise.
   */
  public function cancelTranslation() {
    // Already accepted translation jobs can't be cancelled.
    if ($this->isAccepted()) {
      return FALSE;
    }
    // If the job is not currently pending by the translator (submitted), we can
    // cancel without any checks.
    if (!$this->isSubmitted()) {
      $this->canceled('The translation job has been canceled.');
    }
    else {
      // Otherwise, ask the translator if this job can be cancelled.
      $translator = $this->getTranslatorPlugin();
      $response = $translator->cancelTranslation($this);
      $this->setStateFromResponse($response);
    }
    $this->save();
    return $this->isCancelled();
  }

  /**
   * Returns the source data of all job items.
   *
   * @return
   *   A nested array with the source data where the most upper key is the job
   *   item id.
   */
  public function getSourceData() {
    $data = array();
    foreach ($this->getItems() as $key => $item) {
      $data[$key] = $item->getSourceData();
    }
    return $data;
  }

  /**
   * Store translated data back into the items.
   *
   * @param $data
   *   Partially or complete translated data, the most upper key needs to be
   *   the translation job item id.
   */
  public function addTranslatedData($data) {
    $items = $this->getItems();
    foreach ($data as $key => $value) {
      if (isset($items[$key])) {
        $items[$key]->addTranslatedData($value);
      }
    }
  }

}

/**
 * Entity class for the tmgmt_job_message entity.
 *
 * @ingroup message
 */
class TMGMTJobMessage extends Entity {

  /**
   * The ID of the message..
   *
   * @var integer
   */
  public $tjmid;

  /**
   * The ID of the job.
   *
   * @var integer
   */
  public $tjid;

  /**
   * The message text.
   *
   * @var string
   */
  public $message;

  /**
   * An array of string replacement arguments as used by t().
   *
   * @var array
   */
  public $variables;

  /**
   * The status before the event occurred.
   *
   * @var int
   */
  public $status_before;

  /**
   * The status after the event occurred.
   *
   * @var int
   */
  public $status_after;

  /**
   * The time when the message object was created as a timestamp.
   *
   * @var int
   */
  public $created;

  /**
   * Overrides Entity::__construct().
   */
  public function __construct(array $values = array()) {
    parent::__construct($values, 'tmgmt_job_message');
    $this->created = REQUEST_TIME;
  }

}