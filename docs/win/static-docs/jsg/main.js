$(document).ready(function(){
	$("#side-nav li").mouseover(function(){
		$(this).addClass("active").siblings(".active").removeClass("active");
		var referenceId = $(this).data("reference");
		$("#"+referenceId).addClass("active").siblings(".reference").removeClass("active");
	});
});