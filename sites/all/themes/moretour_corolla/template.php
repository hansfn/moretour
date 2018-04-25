<?php

/**
 * Minor rewrite of theme_username so names aren't truncated.
 */
function moretour_corolla_username($variables) {
  if (!empty($variables['name_raw'])) {
    $name = check_plain($variables['name_raw']); 
  } else {
    $name = $variables['name'];
  }
  if (isset($variables['link_path'])) {
    // We have a link path, so we should generate a link using l().
    // Additional classes may be added as array elements like
    // $variables['link_options']['attributes']['class'][] = 'myclass';
    $output = l($name . $variables['extra'], $variables['link_path'], $variables['link_options']);
  }
  else {
    // Modules may have added important attributes so they must be included
    // in the output. Additional classes may be added as array elements like
    // $variables['attributes_array']['class'][] = 'myclass';
    $output = '<span' . drupal_attributes($variables['attributes_array']) . '>' . $name . $variables['extra'] . '</span>';
  }
  return $output;
}

function moretour_corolla_preprocess(&$variables, $hook) {
  if (($hook != 'page') && ($hook != 'html')) {
    return;
  }

  if ($variables['is_front']) {
    $path = variable_get('site_frontpage');
    $path_alias = drupal_lookup_path('alias',$path);
    if ($path_alias) {
      $path = $path_alias;
    }
  } 
  else {
    $path = request_path();
  }
  if (preg_match('#^(\d{4})(/.*)?#', $path, $matches)) {
    $year = $matches[1];
  }
  else if (preg_match('#^(turnering|blogg)/(\d{4})-(.*)#', $path, $matches)) {
    $year = $matches[2];
  }
  if (!empty($year)) {
    // Append year to the site name/title.
    if ($hook == 'page') {
      $variables['site_name'] .= " $year";
    }
    else {
      $variables['head_title'] .= " $year";
    }
    if (!empty($variables['site_logo'])) {
      if ($year != '2017') { // HFN - TBD
      $variables['site_logo'] = str_replace('NOYEAR', $year, $variables['site_logo']);
      }
    }
  }
}
