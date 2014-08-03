<?php

/**
 * Implements hook_views_query_alter().
 */
function moretour_views_query_alter(&$view, &$query) {
  global $user;
  $current_path = current_path();
  $path = drupal_lookup_path('alias', $current_path);
  if (empty($path)) {
    $path = $current_path;
  }
  // Modify year condition in blogg content page.
  if ($view->name == 'blogg_content_pane') {
    $year = array_shift(explode('/', $path));
    if ($year == 'admin') {
      return;
    }
    $start = strtotime("$year-01-01");
    $end = strtotime("$year-12-31");
    // Traverse through the 'where' part of the query.
    foreach ($query->where as &$condition_group) {
      foreach ($condition_group['conditions'] as $key => $condition) {
        if ($condition['operator'] == 'formula') {
	  $condition_group['conditions'][$key]['field'] = sprintf(
	    'node.created BETWEEN %s AND %s', $start, $end);
	  break;
	}
      }
    }
  }
  // Modify year condition in all ranking queries.
  elseif ($view->name == 'ranking') {
    $year = array_shift(explode('/', $path));
    if ($year == 'admin') {
      return;
    }
    $year_tid = array_shift(array_keys(taxonomy_get_term_by_name($year, 'year')));
    // Traverse through the 'where' part of the query.
    foreach ($query->where as &$condition_group) {
      foreach ($condition_group['conditions'] as $key => $condition) {
	if (strpos($condition['field'], '.field_year_tid') !== false) {
	  $condition_group['conditions'][$key]['value'] = $year_tid;
	  break;
	}
      }
    }
  }
  // Repeating the exposed filter for player 2 (to avoid two selections).
  elseif ($view->name == 'send_mail_for_signups') {
    // Traverse through the 'where' part of the query.
    foreach ($query->where as &$condition_group) {
      if ($condition_group['type'] == 'OR') {
	if (count($condition_group['conditions']) == 2) {
	  $condition_group['conditions'][1] = $condition_group['conditions'][0];
	  $condition_group['conditions'][1]['field'] = 
	    str_replace('field_player_1', 'field_player_2', $condition_group['conditions'][0]['field']);
	} 
	else {
	  unset($condition_group['conditions'][0]);
	}
	break;
      }
    }
  }
  // Repeating the contextual filter for player 2 (and combining them with OR).
  elseif (($view->name == 'signups') || ($view->name == 'results')) {
    // Traverse through the 'where' part of the query.
    foreach ($query->where as &$condition_group) {
      if ($condition_group['conditions'][0]['operator'] == 'formula') {
	$condition_group['conditions'][1] = array();
	$condition_group['conditions'][1]['field'] = 
	  str_replace('field_player_1.field_player_1', 'field_player_2.field_player_2', $condition_group['conditions'][0]['field']);
	$condition_group['conditions'][1]['operator'] = 'formula';
	$condition_group['conditions'][1]['value'] = $condition_group['conditions'][0]['value'];
	$condition_group['type'] = 'OR';
	break;
      }
    }
    /* 
    dpm($view, 'view');
    dpm($query, 'query');
    */
  }
  // Let administrators use/see tournaments where the signup dead-line has passed.
  elseif ($view->name == 'signup_tournament_list') {
    if (!in_array('spiller', $user->roles)) {
      unset($query->where[1]['conditions'][3]);
      // dpm($query);
    }
  }
}

/**
 * Implements hook_views_pre_view().
 */
function moretour_views_pre_view(&$view, &$display_id, &$args) {
  global $user;
  // Let administrators edit also the first player in a signup.
  if (($view->name == 'signup_player_list') && ($display_id == 'references_1')) {
    if (!in_array('spiller', $user->roles)) {
      unset($view->display_handler->options['filters']['uid_current']);
    }
  }
} 


 /**
* Implements hook_views_pre_render().
*/
function moretour_views_pre_render(&$view) {
  // Hack to create ranking since Views doesn't support UNION.
  if (($view->name == 'ranking') && (
    	(strpos($view->current_display, 'page') === 0) || 
	($view->current_display == 'panel_pane_1') || 
	($view->current_display == 'panel_pane_2'))) {

    $other_view = views_get_view('ranking', TRUE); 
    // if ($view->current_display == 'page') {
    if (strpos($view->current_display, 'page') === 0) {
      $other_view->set_display('attachment_1');
    }
    elseif ($view->current_display == 'panel_pane_1') {
      $other_view->set_display('attachment_2');
    }
    elseif ($view->current_display == 'panel_pane_2') {
      $other_view->set_display('attachment_3');
    }
    $other_view->build();
    $other_view->execute();
    $view->result = array_merge($view->result, $other_view->result);
    // Aggregate the results (from each tournament) so each player gets one score.
    $keys = array_keys($view->result);
    foreach ($keys as $key) {
      $result = $view->result[$key];
      unset($view->result[$key]);
      if (empty($view->result[$result->users_name])) {
	$view->result[$result->users_name] = $result;
      } 
      else {
	$value = $view->result[$result->users_name]->field_field_score[0]['raw']['value'];
	$view->result[$result->users_name]->field_field_score[0]['raw']['value'] =
	  $value + $result->field_field_score[0]['raw']['value'];
	$view->result[$result->users_name]->field_field_score[0]['rendered']['#markup'] =
	  $value + $result->field_field_score[0]['raw']['value'];
      }
    }
    usort($view->result, '_moretour_ranking_cmp');
    // If it's one of the "Best player" panes, return maximum 10 players.
    if (($view->current_display == 'panel_pane_1') || ($view->current_display == 'panel_pane_2')) {
      $view->result = array_slice($view->result, 0, 10);
    }
  }
}

function _moretour_ranking_cmp($a, $b) {
  $vala = $a->field_field_score[0]['raw']['value'];
  $valb = $b->field_field_score[0]['raw']['value'];
  if ($vala == $valb) {
    return 0;
  } 
  else {
    return ($vala > $valb) ? -1 : 1;
  }
}
