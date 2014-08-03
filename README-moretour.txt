Klargjøring for ny sesong:

0) Opprett en ny forside.
1) Opprett nytt år på http://moretour.no/admin/structure/taxonomy/year
2) Endre variablen "moretour_current_year_tid" til term-ID-en for det nye året.
   > drush vset moretour_current_year_tid XY
3) Opprett ny nettside for det nye året i følgende visninger:
   - http://moretour.no/admin/structure/views/view/blogg
   - http://moretour.no/admin/structure/views/view/ranking
   - http://moretour.no/admin/structure/views/view/tournamentlist
   Husk å endre sti og år for de nye nettsiden (i visningen)
3) Endre år (year) i følgende visninger:
   - http://moretour.no/admin/structure/views/view/signups
   - http://moretour.no/admin/structure/views/view/signup_tournament_list
   - http://moretour.no/admin/structure/views/view/signups_for_all_tournaments
   - http://moretour.no/admin/structure/views/view/signups_for_result
4) Legg til forrige sesong på 
   - http://moretour.no/side/arkiv
   - http://moretour.no/admin/structure/menu/manage/main-menu
5) Oppdater lenker i hovedmenyen
   - http://moretour.no/admin/structure/menu/manage/main-menu
