(function($) {
  Drupal.behaviors.Pushflew = {
    attach: function (context, settings) {
      var s = document.createElement("script");
      var protocol = settings.Pushflew.protocol;
      var siteId = settings.Pushflew.websiteId;
      if(protocol === "https:"){
        siteId = siteId + "-https";
      }

      s.src = "https://cdn.pushflew.com/cs/"+siteId+".js";
      s.async = true;
      console.log(s);
      //jQuery("head").append(s);
      // document.getElementsByTagName('head')[0].appendChild(s);
      document.documentElement.appendChild(s);
    }
  };

})(jQuery);

