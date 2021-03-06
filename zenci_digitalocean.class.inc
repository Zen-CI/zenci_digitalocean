<?php
/**
 * @file
 * DigitalOcean API wrapper class definition.
 */


use \Curl\Curl;

require_once 'libraries/php-curl-class/src/Curl/CaseInsensitiveArray.php';
require_once 'libraries/php-curl-class/src/Curl/Curl.php';


/**
 * DigitalOcean API wrapper class.
 */
class digitalOceanAPI {

  /**
   * The Curl object used for performing HTTP requests.
   *
   * @var \Curl\Curl
   */
  protected $curl;

  /**
   * Token required to interact with ZenCI API.
   *
   * @var string
   */
  protected $token = '';

  /**
   * Organisation or User name.
   *
   * @var string
   */
  protected $owner_name;

  /**
   * Repository name.
   *
   * @var string
   */
  protected $repo_name;

  /**
   * Debug mode. If enabled, data will be printed via backdrop_set_message().
   *
   * @var bool
   */
  protected $debug = FALSE;

  /**
   * Latest error message from ZenCI API.
   *
   * @var string
   */
  protected $error_message = '';

  /**
   * Constructor.
   */
  public function __construct() {
    $this->reInitCurl();
    $this->setDebug();
  }

  /**
   * Initializate $this->curl with Curl class and preset headers and user agent.
   */
  public function reInitCurl() {
    $this->curl = new Curl();
    $this->curl->setHeader('Content-Type', 'application/json');
    $this->curl->setUserAgent('Backdrop CMS ZenCI DigitalOcean API module');
    $this->curl->setHeader('Accept', '*/*');
  }

  /**
   * Set debug value. False by default.
   *
   * @param $debug boolean
   *   TRUE or FALSE
   */
  public function setDebug($debug = FALSE) {
    $this->debug = $debug;
  }

  /**
   * Set token for DigitalOcean API.
   *
   * @param $token
   *   Token to access DigitalOcean API.
   */
  public function setToken($token = '') {
    $this->token = $token;
  }

  /**
   * Get store token.
   *
   * @return
   *   A string value. Stored token for DigitalOcean API access.
   */
  public function getToken() {
    return $this->token;
  }

  /**
   * Determine if curl request has been falen with error.
   *
   * @return boolean
   *   TRUE or FALSE based on answer from DigitalOcean API.
   */
  public function isError() {
    return $this->curl->curlError or $this->curl->error;
  }

  /**
   * Get Curl details after request.
   *
   * @return array
   *   An array of request information:
   *     - code: the last error number. @see curl_errno.
   *     - message: A clear text error message for the last cURL operation.
   *   @see curl_error.
   *     - request_headers: an array of request headers.
   *     - response_headers: an array of response headers.
   */
  public function testingGetHeaders() {
    return array(
      'code' => $this->curl->curlErrorCode,
      'message' => $this->curl->curlErrorMessage,
      'request_headers' => $this->curl->requestHeaders,
      'response_headers' => $this->curl->responseHeaders,
      'curl' => $this->curl
    );
  }

  /**
   * Get Curl details if error happen.
   *
   * @return
   *   An array of request information. @see testingGetHeaders.
   *   FALSE if there is no error.
   */
  public function getErrors() {
    if ($this->isError()) {
      return $this->testingGetHeaders();
    }
    return FALSE;
  }

  /**
   * Get DigitalOcean APi Error message if error happen.
   *
   * @return
   *   String with error message.
   */
  public function getDigitalOceanErrorMessage() {
    return $this->error_message;
  }

  /**
   * Determine if request has been cached.
   *
   * @return
   *   FALSE if error. Object with answer if request success.
   */
  protected function getResponse() {
    if ($this->debug) {
      backdrop_set_message('<pre>' . print_r($this->testingGetHeaders(), TRUE) . '</pre>');
    }
    if ($this->isError()) {
      if (isset($this->curl->response->error) && $this->curl->response->error) {
        if (isset($this->curl->response->message)) {
          $this->error_message = $this->curl->response->message;
        }
      }

      if ($message = $this->getDigitalOceanErrorMessage()) {
        backdrop_set_message(t('DigitalOcean error message !message', array('!message' => $message)), 'error');
      }
      else {
        $code = $this->curl->errorCode;
        $message = $this->curl->errorMessage;
        backdrop_set_message(t('Communication error: !code !message', array(
          '!code' => $code,
          '!message' => $message
        )), 'error');
      }

      return FALSE;
    }
    if (is_object($this->curl->response)) {
      return (array) $this->curl->response;
    }
    if ('null' == $this->curl->response or NULL == $this->curl->response) {
      return FALSE;
    }
    return $this->curl->response;
  }

  /**
   * Perform GET request to DigitalOcean API and return answer.
   *
   * @param $command
   *   String value. DigitalOcean API url with tokens Like :owner, :repo and ect.
   * @param $params array
   *   Values for request and tokens for request url. Like :owner, :repo, :id
   *   and etc.
   *
   * @return array|bool|null
   *   FALSE if request failed. An array of data on success.
   */
  protected function getRequest($command, $params = array()) {

    $this->prepareCommand($command, $params);

    if ($this->getToken()) {
      $this->curl->setHeader('Authorization', 'Bearer ' . $this->getToken());
    }

    $this->curl->get(ZENCI_DIGITALOCEAN_API_URI . '/' . $command, $params);
    $response = $this->getResponse();
    return $response;
  }

  /**
   * Perform PUT request to DigitalOcean API and return answer.
   *
   * @param $command
   *   String value. DigitalOcean API url with tokens Like :owner, :repo and ect.
   * @param $params array
   *   Values for request and tokens for request url. Like :owner, :repo, :id
   *   and etc.
   *
   * @return
   *   FALSE if request failed. Object if success.
   */
  protected function putRequest($command, $params = array()) {

    $this->prepareCommand($command, $params);

    if ($this->getToken()) {
      $this->curl->setHeader('Token', $this->getToken());
    }

    $this->curl->put(ZENCI_DIGITALOCEAN_API_URI . '/' . $command, $params);
    $response = $this->getResponse();
    return $response;
  }

  /**
   * Perform POST request to DigitalOcean API and return answer.
   *
   * @param $command
   *   String value. DigitalOcean API url with tokens Like :owner, :repo and ect.
   * @param $params array
   *   Values for request and tokens for request url. Like :owner, :repo, :id
   *   and etc.
   *
   * @return
   *   FALSE if request failed. Object if success.
   */
  protected function postRequest($command, $params = array()) {

    $this->prepareCommand($command, $params);
    if ($this->getToken()) {
      $this->curl->setHeader('Authorization', 'Bearer ' . $this->getToken());
    }
    $this->curl->post(ZENCI_DIGITALOCEAN_API_URI . '/' . $command, $params);
    $response = $this->getResponse();
    return $response;
  }

  /**
   * Perform DELETE request to DigitalOcean API and return answer.
   *
   * @param $command
   *   String value. DigitalOcean API url with tokens Like :owner, :repo and ect.
   * @param $params array
   *   Values for request and tokens for request url. Like :owner, :repo, :id
   *   and etc.
   *
   * @return
   *   FALSE if request failed. Object if success.
   */
  protected function deleteRequest($command, $params = array()) {

    $this->prepareCommand($command, $params);

    if ($this->getToken()) {
      $this->curl->setHeader('Authorization', 'Bearer ' . $this->getToken());
    }

    $this->curl->delete(ZENCI_DIGITALOCEAN_API_URI . '/' . $command, $params);
    $response = $this->getResponse();
    return $response;
  }

  /**
   * Replace tokens with values in request url.
   *
   * @param $command
   *   String value. DigitalOcean API url with tokens Like :owner, :repo and ect.
   * @param $params array
   *   Values for request and tokens for request url. Like :owner, :repo, :id
   *   and etc.
   */
  private function prepareCommand(&$command, &$params) {
    foreach ($params as $key => $val) {
      if ($key[0] == ':') {
        $command = str_replace($key, $val, $command);
        unset($params[$key]);
      }
    }
  }

  /**
   * Get Regions information.
   * https://developers.digitalocean.com/documentation/v2/#regions
   *
   * @return
   *   Return regions information or FALSE if there is no regions or no
   *   access.
   *
   */
  public function getRegions() {
    $answer = $this->getRequest('regions');
    if(isset($answer['regions'])){
      return $answer['regions'];
    }
    return $answer;
  }

  /**
   * Get Images information.
   * https://developers.digitalocean.com/documentation/v2/#images
   *
   * @return
   *   Return images information or FALSE if there is no images or no
   *   access.
   *
   */
  public function getImages($private = FALSE) {
    $params = array();
    if($private){
      $params['private'] = 'true';
    };
    $answer = $this->getRequest('images', $params);
    if(isset($answer['images'])){
      return $answer['images'];
    }
    return $answer;
  }

  /**
   * Get Image information.
   * https://developers.digitalocean.com/documentation/v2/#retrieve-an-existing-image-by-id
   *
   * @return
   *   Return images information or FALSE if there is no images or no
   *   access.
   *
   */
  public function getImage($image_id) {

    $answer = $this->getRequest('images/' . $image_id);
    if(isset($answer['image'])){
      return $answer['image'];
    }
    return $answer;
  }
  
  /**
   * Get Size information.
   * https://developers.digitalocean.com/documentation/v2/#sizes
   *
   * @return
   *   Return size information or FALSE if there is no sizes or no
   *   access.
   *
   */
  public function getSizes() {
    $answer = $this->getRequest('sizes');
    if(isset($answer['sizes'])){
      return $answer['sizes'];
    }
    return $answer;
  }
  /**
   * Create droplet.
   * https://developers.digitalocean.com/documentation/v2/#create-a-new-droplet
   *
   * @param $settings array
   *   An array of settings for droplet
   *     - name: The name for droplet. REQUIRED
   *     - region: The region for droplet.  REQUIRED
   *     - size: The size for droplet.  REQUIRED
   *     - image: The image for droplet.  REQUIRED
   *     - ssh_keys: The ssh_keys for droplet.
   *     - backups: The backups for droplet.
   *     - ipv6: The ipv6 for droplet. TRUE or FALSE
   *     - user_data: The user_data for droplet.
   *     - private_networking: The private_networking for droplet.
   *
   * @return
   *   Return droplet information or FALSE if there is no access.
   *
   */
   
  /**
   * Get Image information.
   * https://developers.digitalocean.com/documentation/v2/#retrieve-an-existing-image-by-id
   *
   * @return
   *   Return images information or FALSE if there is no images or no
   *   access.
   *
   */
  public function getDroplet($droplet_id) {

    $answer = $this->getRequest('droplets/' . $droplet_id);
    if(isset($answer['droplet'])){
      return $answer['droplet'];
    }
    return $answer;
  }
  
  public function createDroplet($settings) {
    $answer =  $this->postRequest('droplets', $settings);
    if(isset($answer['droplet'])){
      return $answer['droplet'];
    }
    return $answer;

  }

  /**
   * Update hook for repository.
   *
   * @param $settings array
   *   An array of settings for hook
   *     - url: The URL to which the payloads will be delivered.
   *     - secret: If provided, payloads will be delivered with an Signature
   *   header.
   *
   * @return
   *   Return hook information or FALSE if there is no access.
   *
   * @see SetRepoName
   * @see SetOwnerName
   */
  public function updateHook($settings) {
    $settings[':owner'] = $this->owner_name;
    $settings[':repo'] = $this->repo_name;
    return $this->putRequest('repos/:owner/:repo/hook', $settings);
  }

  /**
   * Delete hook for repository.
   * @return
   *   Return empty value if success or FALSE if error.
   *
   * @see SetRepoName
   * @see SetOwnerName
   */
  public function deleteHook() {
    return $this->deleteRequest(
      'repos/:owner/:repo/hook', array(
        ':owner' => $this->owner_name,
        ':repo' => $this->repo_name,
      )
    );
  }
}
