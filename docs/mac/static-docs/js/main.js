function showTab(id) {
  for (var i = 1; i <= 6; i++) {
    var el = document.getElementById("reference-"+i);
    var li = document.getElementById("li-reference-"+i);

    if (!el) continue;
    if (id == el.id) {
      el.className = 'reference active';
      li.className = 'active';
    } else {
      el.className ='reference';
      li.className = '';
    }
  }
}