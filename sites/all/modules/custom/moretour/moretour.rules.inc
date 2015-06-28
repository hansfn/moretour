<?php
/*
 * Implements hook_rules_action_info.
 */
function moretour_rules_action_info() {
  return array(
    'moretour_set_points_tournament_action_2011' => array(
      'label' => t('Sets ranking points for all places in a tournament - 2011 version'),
      'group' => t('Møre Tour'),
      'parameter' => array(
        'tournament' => array(
          'label' => t('Tournament'),
          'type' => 'node'
        )
      )
    ),
    'moretour_set_points_tournament_action' => array(
      'label' => t('Sets ranking points for all places in a tournament'),
      'group' => t('Møre Tour'),
      'parameter' => array(
        'tournament' => array(
          'label' => t('Tournament (with signups)'),
          'type' => 'node'
        )
      )
    )
  );
}

/*
 * Sets ranking points for all places in a tournament - 2011 version.
 */
function moretour_set_points_tournament_action_2011($node) {
  global $user;

  // Abort immediately if node/tournament isn't published.
  // FIXME: If the tournament was unpublished, we should delete field collections for 
  // this tournament on all players and update total ranking points.
  if ($node->status != 1) {
    return;
  }

  // Ranking point matrix.
  $points = array();
  $points[4]  = array(25, 23, 20, 18);
  $points[5]  = array(31, 28, 25, 22, 19);
  $points[6]  = array(38, 34, 30, 26, 23, 21);
  $points[7]  = array(44, 39, 35, 31, 26, 24, 22);
  $points[8]  = array(50, 45, 40, 35, 30, 28, 25, 23);
  $points[9]  = array(56, 51, 45, 39, 34, 31, 28, 25, 23);
  $points[10] = array(63, 56, 50, 44, 38, 34, 31, 28, 25, 22);
  $points[11] = array(69, 62, 55, 48, 41, 38, 34, 31, 28, 24, 21);
  $points[12] = array(75, 68, 60, 53, 45, 41, 38, 34, 30, 26, 23, 19);
  $points[13] = array(81, 73, 65, 57, 49, 45, 41, 37, 33, 28, 24, 20, 16);
  $points[14] = array(88, 79, 70, 61, 53, 48, 44, 39, 35, 31, 26, 22, 18, 13);
  $points[15] = array(94, 84, 75, 66, 56, 52, 47, 42, 38, 33, 28, 23, 19, 14, 9);
  $points[16] = array(100, 90, 80, 70, 60, 55, 50, 45, 40, 35, 30, 25, 20, 15, 10, 5);

  // Factor used to modify points for play-offs ("finale").
  $playoff_factor = 2;

  // Modify ranking point matrix if this a play-off (finale).
  if ($node->field_playoff[LANGUAGE_NONE][0]['value']) {
    function _set_playoff_points(&$value, $key, $factor) {
      $value *= $factor;
    }
    array_walk_recursive($points, "_set_playoff_points", $playoff_factor);
  }

  // FIXME: Make this a visible config option.
  $current_year_tid = 1; // 2011. 

  $lang = $node->language;

  // Only update ranking points (and field collections) for current year.
  if ($node->field_year[$lang][0]['tid'] != $current_year_tid) {
    drupal_set_message("Rankingpoeng ikke oppdatert siden denne turneringen ikke er med i årets sesong. ");
    return;
  }

  foreach (array('men','women') as $sex) {
    $cont = true;
    $place = 1;
    $team_count = 0;
    // Determine the team count
    while ($cont) {
      $field_name = 'field_place_' . $place . '_' . $sex;
      $field = $node->{$field_name};
      if (empty($field)) {
	break;
      }
      switch (count($field[$lang])) {
      	case 0:
	  $cont = false;
	  break;
	case 2:
	  $team_count++;
	  $place++;
	  break;
	case 4:
	  $team_count += 2;
	  $place += 2;
	  break;
	case 6:
	  $team_count += 3;
	  $place += 3;
	  break;
	case 8:
	  $team_count += 4;
	  $place += 4;
	  break;
	default:
	  $cont = false;
	  drupal_set_message("Feil - antall lag for plass $place i klassen $sex er ikke partall!");
	  break;
      }
      $field_name = 'field_team_count_' . $sex;
      $node->{$field_name}[$lang][0] = array('value' => $team_count);
    }
    // Nuke old ranking points:
    for ($i = 1; ; $i++) {
      $field_points_name = 'field_points_' . $i . '_' . $sex;
      if (!isset($node->{$field_points_name})) {
	break;
      } else {
	$node->{$field_points_name}[$lang] = array();
      }
    }
    if ($team_count < 4) {
      continue;
    }
    // Determine the actual ranking point for each place/position
    // PS! We are currently storing these values so you can check the calculations ...
    $cont = true;
    $place = 1;
    while ($cont) {
      $field_points_name = 'field_points_' . $place . '_' . $sex;
      $field_name = 'field_place_' . $place . '_' . $sex;
      $field = $node->{$field_name};
      if (empty($field)) {
	break;
      }
      switch (count($field[$lang])) {
      	case 0:
	  $cont = false;
	  break;
	case 2:
	  $value = $points[$team_count][$place-1];
	  $node->{$field_points_name}[$lang][0] = array('value' => $value);
	  $place++;
	  break;
	case 3:
	case 4:
	  $value = $points[$team_count][$place-1];
	  $value += $points[$team_count][$place];
	  $value = floor($value/2.0);
	  $node->{$field_points_name}[$lang][0] = array('value' => $value);
	  $place += 2;
	  break;
	case 5:
	case 6:
	  $value = $points[$team_count][$place-1];
	  $value += $points[$team_count][$place];
	  $value += $points[$team_count][$place+1];
	  $value = floor($value/3.0);
	  $node->{$field_points_name}[$lang][0] = array('value' => $value);
	  $place += 3;
	  break;
	case 7:
	case 8:
	  $value = $points[$team_count][$place-1];
	  $value += $points[$team_count][$place];
	  $value += $points[$team_count][$place+1];
	  $value += $points[$team_count][$place+2];
	  $value = floor($value/4.0);
	  $node->{$field_points_name}[$lang][0] = array('value' => $value);
	  $place += 4;
	  break;
      }
      // Create/update field collection for this tournament on all players.
      // Also update total ranking points on all players.
      // Code inspired by http://drupal.org/node/1158602 and http://drupal.org/node/1179088 - thx!
      foreach ($field[$lang] as $player ) {
	$player_node = node_load($player['nid']);
	$fc_exists = false;
	$totalvalue = 0;
	if (!empty($player_node->field_player_points_with_tour)) {
	  foreach ($player_node->field_player_points_with_tour[LANGUAGE_NONE] as $temp_item) {
	    $fc_item = reset(entity_load('field_collection_item', array($temp_item['value'])));
	    $fc_item_year_tid = $fc_item->field_year[LANGUAGE_NONE][0]['tid'];
	    $fc_item_tour_nid = $fc_item->field_player_pwt_tourref[LANGUAGE_NONE][0]['nid'];
	    $fc_item_points_value = $fc_item->field_player_pwt_points[LANGUAGE_NONE][0]['value'];
	    // Skip field collections for prior years.
	    if ($fc_item_year_tid != $current_year_tid) {
	      continue;
	    }
	    if ($fc_item_tour_nid == $node->nid) {
	      // This tournament is already added - update points in field collection and save it.
	      $fc_exists = true;
	      $totalvalue += $value;
	      $fc_item->field_player_pwt_points[LANGUAGE_NONE][0]['value'] = $value;
	      $fc_item->save();
	    } else {
	      $totalvalue += $fc_item_points_value;
	    }
	  }
	}

	// Add the field collection for this tournament on this player (if it doesn't exist).
	if (!$fc_exists) {
	  $fc_item = entity_create('field_collection_item', array('field_name' => 'field_player_points_with_tour'));
	  $fc_item->setHostEntity('node', $player_node);
	  $fc_item->field_year = array(LANGUAGE_NONE => array('0' => array('tid' => $current_year_tid)));
	  $fc_item->field_player_pwt_tourref = array(LANGUAGE_NONE => array('0' => array('nid' => $node->nid)));
	  $fc_item->field_player_pwt_points = array(LANGUAGE_NONE => array('0' => array('value' => $value)));
	  $fc_item->save();
	  $totalvalue += $value;
	  /* It seems saving the field collection updates the host - no need for this line.
	  $player_node->field_player_points_with_tour[LANGUAGE_NONE][] = array('value' => $fc_item->item_id);
	  */
	}

	// Finally setting the total ranking points.
	$player_node->field_player_points_total[$lang][0] = array('value' => $totalvalue);
	node_save($player_node);
      }
    }
  }
}

/*
 * Sets ranking points for all places in a tournament.
 */
function moretour_set_points_tournament_action($node) {
  global $user;

  // Abort immediately if node/tournament isn't published.
  // FIXME: If the tournament was unpublished, we should delete field collections for 
  // this tournament on all players and update total ranking points.
  if ($node->status != 1) {
    return;
  }

  // Ranking point matrix.
  $points = array();
  $points[3]  = array(20, 15, 10);
  $points[4]  = array(25, 23, 20, 18);
  $points[5]  = array(31, 28, 25, 22, 19);
  $points[6]  = array(38, 34, 30, 26, 23, 21);
  $points[7]  = array(44, 39, 35, 31, 26, 24, 22);
  $points[8]  = array(50, 45, 40, 35, 30, 28, 25, 23);
  $points[9]  = array(56, 51, 45, 39, 34, 31, 28, 25, 23);
  $points[10] = array(63, 56, 50, 44, 38, 34, 31, 28, 25, 22);
  $points[11] = array(69, 62, 55, 48, 41, 38, 34, 31, 28, 24, 21);
  $points[12] = array(75, 68, 60, 53, 45, 41, 38, 34, 30, 26, 23, 19);
  $points[13] = array(81, 73, 65, 57, 49, 45, 41, 37, 33, 28, 24, 20, 16);
  $points[14] = array(88, 79, 70, 61, 53, 48, 44, 39, 35, 31, 26, 22, 18, 13);
  $points[15] = array(94, 84, 75, 66, 56, 52, 47, 42, 38, 33, 28, 23, 19, 14, 9);
  $points[16] = array(100, 90, 80, 70, 60, 55, 50, 45, 40, 35, 30, 25, 20, 15, 10, 5);
  $points[17] = array(107, 96, 85, 74, 64, 58, 53, 48, 43, 38, 33, 28, 23, 18, 13, 8, 6);
  $points[18] = array(114, 102, 90, 78, 68, 61, 56, 51, 46, 41, 36, 31, 25, 20, 15, 10, 8, 6);
  $points[19] = array(121, 108, 95, 82, 72, 64, 59, 54, 49, 44, 39, 34, 27, 22, 17, 12, 10, 8, 6);
  $points[20] = array(128, 114, 100, 86, 76, 67, 62, 57, 52, 47, 42, 37, 29, 24, 19, 14, 12, 10, 8, 6);

  // Factor used to modify points for play-offs ("finale").
  $playoff_factor = 2;

  // Modify ranking point matrix if this a play-off (finale).
  if ($node->field_playoff[LANGUAGE_NONE][0]['value']) {
    function _set_playoff_points(&$value, $key, $factor) {
      $value *= $factor;
    }
    array_walk_recursive($points, "_set_playoff_points", $playoff_factor);
  }

  // FIXME: Make this a visible config option.
  $current_year_tid = variable_get('moretour_current_year_tid');

  $lang = $node->language;

  // Only update ranking points for current year.
  if ($node->field_year[$lang][0]['tid'] != $current_year_tid) {
    drupal_set_message("Rankingpoeng ikke oppdatert siden denne turneringen ikke er med i årets sesong.");
    return;
  }

  foreach (array('men','women') as $sex) {
    $cont = true;
    $place = 1;
    $team_count = 0;
    // Determine the team count
    while ($cont) {
      $field_name = 'field_place_' . $place . '_' . $sex;
      $field = $node->{$field_name};
      if (empty($field)) {
	break;
      }
      switch (count($field[$lang])) {
      	case 0:
	  $cont = false;
	  break;
	case 1:
	  $team_count++;
	  $place++;
	  break;
	case 2:
	  $team_count += 2;
	  $place += 2;
	  break;
	case 3:
	  $team_count += 3;
	  $place += 3;
	  break;
	case 4:
	  $team_count += 4;
	  $place += 4;
	  break;
      }
      $field_name = 'field_team_count_' . $sex;
      $node->{$field_name}[$lang][0] = array('value' => $team_count);
    }
    if ($team_count < 3) {
      // Reset scores to zero.
      $query = new EntityFieldQuery();
      $query->entityCondition('entity_type', 'node')
	->entityCondition('bundle', 'signup')
	->propertyCondition('status', 1)
	->fieldCondition('field_player_sex', 'value', (($sex=='men')?'male':'female'), '=')
	->fieldCondition('field_score', 'value', 0, '>')
	->fieldCondition('field_year', 'tid', $current_year_tid, '=')
	->fieldCondition('field_tournament', 'nid', $node->nid, '=');
      $result = $query->execute();
      if (isset($result['node'])) {
	$signup_nodes = entity_load('node', array_keys($result['node']));
	foreach ($signup_nodes as $signup_node) {
	  $signup_node->field_score[LANGUAGE_NONE][0] = array('value' => 0);
	  node_save($signup_node);
	}
      }

      continue;
    }
    // Determine the actual ranking point for each place/position
    $cont = true;
    $place = 1;
    while ($cont) {

      $curr_place = $place;
      $field_name = 'field_place_' . $place . '_' . $sex;
      $field = $node->{$field_name};
      if (empty($field)) {
	break;
      }
      switch (count($field[$lang])) {
      	case 0:
	  $cont = false;
	  break;
	case 1:
	  $value = $points[$team_count][$place-1];
	  $place++;
	  break;
	case 2:
	  $value = $points[$team_count][$place-1];
	  $value += $points[$team_count][$place];
	  $value = floor($value/2.0);
	  $place += 2;
	  break;
	case 3:
	  $value = $points[$team_count][$place-1];
	  $value += $points[$team_count][$place];
	  $value += $points[$team_count][$place+1];
	  $value = floor($value/3.0);
	  $place += 3;
	  break;
	case 4:
	  $value = $points[$team_count][$place-1];
	  $value += $points[$team_count][$place];
	  $value += $points[$team_count][$place+1];
	  $value += $points[$team_count][$place+2];
	  $value = floor($value/4.0);
	  $place += 4;
	  break;
      }
      // Create/update the corresponding signup.
      foreach ($field[$lang] as $signup ) {
	$signup_node = node_load($signup['nid']);
	$signup_node->field_score[LANGUAGE_NONE][0] = array('value' => $value);
	$signup_node->field_place[LANGUAGE_NONE][0] = array('value' => $curr_place);
	node_save($signup_node);
      }
    }
  }
}
