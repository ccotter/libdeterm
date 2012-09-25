  


<!DOCTYPE html>
<html>
  <head prefix="og: http://ogp.me/ns# fb: http://ogp.me/ns/fb# githubog: http://ogp.me/ns/fb/githubog#">
    <meta charset='utf-8'>
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
        <title>libdeterm/inc/fs_internal.h at master · ccotter/libdeterm · GitHub</title>
    <link rel="search" type="application/opensearchdescription+xml" href="/opensearch.xml" title="GitHub" />
    <link rel="fluid-icon" href="https://github.com/fluidicon.png" title="GitHub" />
    <link rel="apple-touch-icon-precomposed" sizes="57x57" href="apple-touch-icon-114.png" />
    <link rel="apple-touch-icon-precomposed" sizes="114x114" href="apple-touch-icon-114.png" />
    <link rel="apple-touch-icon-precomposed" sizes="72x72" href="apple-touch-icon-144.png" />
    <link rel="apple-touch-icon-precomposed" sizes="144x144" href="apple-touch-icon-144.png" />

    
    
    <link rel="icon" type="image/x-icon" href="/favicon.png" />

    <meta content="authenticity_token" name="csrf-param" />
<meta content="Fm96w+PUsYQ00/ME6td7C9EiiEyIHcqPCyHgPesUHPw=" name="csrf-token" />

    <link href="https://a248.e.akamai.net/assets.github.com/assets/github-796faf7daca344ef5dff71a6ed00c57b88a5f613.css" media="screen" rel="stylesheet" type="text/css" />
    <link href="https://a248.e.akamai.net/assets.github.com/assets/github2-8fbba53285e1ba0adf5aaa39e8b0d8ab6c91d88b.css" media="screen" rel="stylesheet" type="text/css" />
    


    <script src="https://a248.e.akamai.net/assets.github.com/assets/frameworks-ac63e4a1fc7b5030f8c99d5200722f51ed8e7baa.js" type="text/javascript"></script>
    <script defer="defer" src="https://a248.e.akamai.net/assets.github.com/assets/github-b4c07f78e6c6bbda967b3071f067d7fe7587ab18.js" type="text/javascript"></script>
    

      <link rel='permalink' href='/ccotter/libdeterm/blob/dcc51ee8816a571258efd27bb8e9d04cd717603e/inc/fs_internal.h'>
    <meta property="og:title" content="libdeterm"/>
    <meta property="og:type" content="githubog:gitrepository"/>
    <meta property="og:url" content="https://github.com/ccotter/libdeterm"/>
    <meta property="og:image" content="https://a248.e.akamai.net/assets.github.com/images/gravatars/gravatar-user-420.png?1345673561"/>
    <meta property="og:site_name" content="GitHub"/>
    <meta property="og:description" content="C runtime library for linux-deterministic. Contribute to libdeterm development by creating an account on GitHub."/>

    <meta name="description" content="C runtime library for linux-deterministic. Contribute to libdeterm development by creating an account on GitHub." />
  <link href="https://github.com/ccotter/libdeterm/commits/master.atom" rel="alternate" title="Recent Commits to libdeterm:master" type="application/atom+xml" />

  </head>


  <body class="logged_out page-blob  vis-public env-production ">
    <div id="wrapper">

    
    

      <div id="header" class="true clearfix">
        <div class="container clearfix">
          <a class="site-logo " href="https://github.com/">
            <img alt="GitHub" class="github-logo-4x" height="30" src="https://a248.e.akamai.net/assets.github.com/images/modules/header/logov7@4x.png?1337118066" />
            <img alt="GitHub" class="github-logo-4x-hover" height="30" src="https://a248.e.akamai.net/assets.github.com/images/modules/header/logov7@4x-hover.png?1337118066" />
          </a>


                  <!--
      make sure to use fully qualified URLs here since this nav
      is used on error pages on other domains
    -->
    <ul class="top-nav logged_out">
        <li class="pricing"><a href="https://github.com/plans">Signup and Pricing</a></li>
        <li class="explore"><a href="https://github.com/explore">Explore GitHub</a></li>
      <li class="features"><a href="https://github.com/features">Features</a></li>
        <li class="blog"><a href="https://github.com/blog">Blog</a></li>
      <li class="login"><a href="https://github.com/login?return_to=%2Fccotter%2Flibdeterm%2Fblob%2Fmaster%2Finc%2Ffs_internal.h">Sign in</a></li>
    </ul>



          
        </div>
      </div>

      

      


            <div class="site hfeed" itemscope itemtype="http://schema.org/WebPage">
      <div class="container hentry">
        
        <div class="pagehead repohead instapaper_ignore readability-menu">
        <div class="title-actions-bar">
          


              <ul class="pagehead-actions">


          <li>
            <span class="star-button"><a href="/login?return_to=%2Fccotter%2Flibdeterm" class="minibutton js-toggler-target entice tooltipped leftwards" title="You must be signed in to use this feature" rel="nofollow"><span class="mini-icon mini-icon-star"></span>Star</a><a class="social-count js-social-count" href="/ccotter/libdeterm/stargazers">1</a></span>
          </li>
          <li>
            <a href="/login?return_to=%2Fccotter%2Flibdeterm" class="minibutton js-toggler-target fork-button entice tooltipped leftwards"  title="You must be signed in to fork a repository" rel="nofollow"><span class="mini-icon mini-icon-fork"></span>Fork</a><a href="/ccotter/libdeterm/network" class="social-count">0</a>
          </li>
    </ul>

          <h1 itemscope itemtype="http://data-vocabulary.org/Breadcrumb" class="entry-title public">
            <span class="repo-label"><span>public</span></span>
            <span class="mega-icon mega-icon-public-repo"></span>
            <span class="author vcard">
              <a href="/ccotter" class="url fn" itemprop="url" rel="author">
              <span itemprop="title">ccotter</span>
              </a></span> /
            <strong><a href="/ccotter/libdeterm" class="js-current-repository">libdeterm</a></strong>
          </h1>
        </div>

          

  <ul class="tabs">
    <li><a href="/ccotter/libdeterm" class="selected" highlight="repo_sourcerepo_downloadsrepo_commitsrepo_tagsrepo_branches">Code</a></li>
    <li><a href="/ccotter/libdeterm/network" highlight="repo_network">Network</a></li>
    <li><a href="/ccotter/libdeterm/pulls" highlight="repo_pulls">Pull Requests <span class='counter'>0</span></a></li>

      <li><a href="/ccotter/libdeterm/issues" highlight="repo_issues">Issues <span class='counter'>1</span></a></li>



    <li><a href="/ccotter/libdeterm/graphs" highlight="repo_graphsrepo_contributors">Graphs</a></li>


  </ul>
  
<div class="frame frame-center tree-finder" style="display:none"
      data-tree-list-url="/ccotter/libdeterm/tree-list/dcc51ee8816a571258efd27bb8e9d04cd717603e"
      data-blob-url-prefix="/ccotter/libdeterm/blob/dcc51ee8816a571258efd27bb8e9d04cd717603e"
    >

  <div class="breadcrumb">
    <span class="bold"><a href="/ccotter/libdeterm">libdeterm</a></span> /
    <input class="tree-finder-input js-navigation-enable" type="text" name="query" autocomplete="off" spellcheck="false">
  </div>

    <div class="octotip">
      <p>
        <a href="/ccotter/libdeterm/dismiss-tree-finder-help" class="dismiss js-dismiss-tree-list-help" title="Hide this notice forever" rel="nofollow">Dismiss</a>
        <span class="bold">Octotip:</span> You've activated the <em>file finder</em>
        by pressing <span class="kbd">t</span> Start typing to filter the
        file list. Use <span class="kbd badmono">↑</span> and
        <span class="kbd badmono">↓</span> to navigate,
        <span class="kbd">enter</span> to view files.
      </p>
    </div>

  <table class="tree-browser" cellpadding="0" cellspacing="0">
    <tr class="js-header"><th>&nbsp;</th><th>name</th></tr>
    <tr class="js-no-results no-results" style="display: none">
      <th colspan="2">No matching files</th>
    </tr>
    <tbody class="js-results-list js-navigation-container">
    </tbody>
  </table>
</div>

<div id="jump-to-line" style="display:none">
  <h2>Jump to Line</h2>
  <form accept-charset="UTF-8">
    <input class="textfield" type="text">
    <div class="full-button">
      <button type="submit" class="classy">
        Go
      </button>
    </div>
  </form>
</div>


<div class="tabnav">

  <span class="tabnav-right">
    <ul class="tabnav-tabs">
      <li><a href="/ccotter/libdeterm/tags" class="tabnav-tab" highlight="repo_tags">Tags <span class="counter blank">0</span></a></li>
      <li><a href="/ccotter/libdeterm/downloads" class="tabnav-tab" highlight="repo_downloads">Downloads <span class="counter blank">0</span></a></li>
    </ul>
    
  </span>

  <div class="tabnav-widget scope">

    <div class="context-menu-container js-menu-container js-context-menu">
      <a href="#"
         class="minibutton bigger switcher js-menu-target js-commitish-button btn-branch repo-tree"
         data-hotkey="w"
         data-master-branch="master"
         data-ref="master">
         <span><em class="mini-icon mini-icon-branch"></em><i>branch:</i> master</span>
      </a>

      <div class="context-pane commitish-context js-menu-content">
        <a href="javascript:;" class="close js-menu-close"><span class="mini-icon mini-icon-remove-close"></span></a>
        <div class="context-title">Switch branches/tags</div>
        <div class="context-body pane-selector commitish-selector js-navigation-container">
          <div class="filterbar">
            <input type="text" id="context-commitish-filter-field" class="js-navigation-enable" placeholder="Filter branches/tags" data-filterable />
            <ul class="tabs">
              <li><a href="#" data-filter="branches" class="selected">Branches</a></li>
              <li><a href="#" data-filter="tags">Tags</a></li>
            </ul>
          </div>

          <div class="js-filter-tab js-filter-branches" data-filterable-for="context-commitish-filter-field" data-filterable-type=substring>
            <div class="no-results js-not-filterable">Nothing to show</div>
              <div class="commitish-item branch-commitish selector-item js-navigation-item js-navigation-target ">
                <span class="mini-icon mini-icon-confirm"></span>
                <h4>
                    <a href="/ccotter/libdeterm/blob/32bit/inc/fs_internal.h" class="js-navigation-open" data-name="32bit" rel="nofollow">32bit</a>
                </h4>
              </div>
              <div class="commitish-item branch-commitish selector-item js-navigation-item js-navigation-target selected">
                <span class="mini-icon mini-icon-confirm"></span>
                <h4>
                    <a href="/ccotter/libdeterm/blob/master/inc/fs_internal.h" class="js-navigation-open" data-name="master" rel="nofollow">master</a>
                </h4>
              </div>
              <div class="commitish-item branch-commitish selector-item js-navigation-item js-navigation-target ">
                <span class="mini-icon mini-icon-confirm"></span>
                <h4>
                    <a href="/ccotter/libdeterm/blob/x86_64/inc/fs_internal.h" class="js-navigation-open" data-name="x86_64" rel="nofollow">x86_64</a>
                </h4>
              </div>
          </div>

          <div class="js-filter-tab js-filter-tags" style="display:none" data-filterable-for="context-commitish-filter-field" data-filterable-type=substring>
            <div class="no-results js-not-filterable">Nothing to show</div>
          </div>
        </div>
      </div><!-- /.commitish-context-context -->
    </div>
  </div> <!-- /.scope -->

  <ul class="tabnav-tabs">
    <li><a href="/ccotter/libdeterm" class="selected tabnav-tab" highlight="repo_source">Files</a></li>
    <li><a href="/ccotter/libdeterm/commits/master" class="tabnav-tab" highlight="repo_commits">Commits</a></li>
    <li><a href="/ccotter/libdeterm/branches" class="tabnav-tab" highlight="repo_branches" rel="nofollow">Branches <span class="counter ">3</span></a></li>
  </ul>

</div>

  
  
  


          

        </div><!-- /.repohead -->

        <div id="js-repo-pjax-container" data-pjax-container>
          


<!-- blob contrib key: blob_contributors:v21:3f18a53b7c922ae47ad464b4092a7a23 -->
<!-- blob contrib frag key: views10/v8/blob_contributors:v21:3f18a53b7c922ae47ad464b4092a7a23 -->

<!-- block_view_fragment_key: views10/v8/blob:v21:67dc4bd39482fc7bbd7c733e1a42465f -->

  <div id="slider">

    <div class="breadcrumb" data-path="inc/fs_internal.h/">
      <b itemscope="" itemtype="http://data-vocabulary.org/Breadcrumb"><a href="/ccotter/libdeterm/tree/dcc51ee8816a571258efd27bb8e9d04cd717603e" class="js-rewrite-sha" itemprop="url"><span itemprop="title">libdeterm</span></a></b> / <span itemscope="" itemtype="http://data-vocabulary.org/Breadcrumb"><a href="/ccotter/libdeterm/tree/dcc51ee8816a571258efd27bb8e9d04cd717603e/inc" class="js-rewrite-sha" itemscope="url"><span itemprop="title">inc</span></a></span> / <strong class="final-path">fs_internal.h</strong> <span class="js-clippy mini-icon mini-icon-clippy " data-clipboard-text="inc/fs_internal.h" data-copied-hint="copied!" data-copy-hint="copy to clipboard"></span>
    </div>

      
  <div class="commit file-history-tease js-blob-contributors-content" data-path="inc/fs_internal.h/">
    <img class="main-avatar" height="24" src="https://secure.gravatar.com/avatar/31bc77433a0dac00bc0a57fdaba847ae?s=140&amp;d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png" width="24" />
    <span class="author"><a href="/ccotter">ccotter</a></span>
    <time class="js-relative-date" datetime="2012-07-29T13:10:28-07:00" title="2012-07-29 13:10:28">July 29, 2012</time>
    <div class="commit-title">
        <a href="/ccotter/libdeterm/commit/a92fdd7e83c1197f0957532a5578a57b3d4c53ff" class="message">Initial commit</a>
    </div>

    <div class="participation">
      <p class="quickstat"><a href="#blob_contributors_box" rel="facebox"><strong>1</strong> contributor</a></p>
      
    </div>
    <div id="blob_contributors_box" style="display:none">
      <h2>Users on GitHub who have contributed to this file</h2>
      <ul class="facebox-user-list">
        <li>
          <img height="24" src="https://secure.gravatar.com/avatar/31bc77433a0dac00bc0a57fdaba847ae?s=140&amp;d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png" width="24" />
          <a href="/ccotter">ccotter</a>
        </li>
      </ul>
    </div>
  </div>


    <div class="frames">
      <div class="frame frame-center" data-path="inc/fs_internal.h/" data-permalink-url="/ccotter/libdeterm/blob/dcc51ee8816a571258efd27bb8e9d04cd717603e/inc/fs_internal.h" data-title="libdeterm/inc/fs_internal.h at master · ccotter/libdeterm · GitHub" data-type="blob">

        <div id="files" class="bubble">
          <div class="file">
            <div class="meta">
              <div class="info">
                <span class="icon"><b class="mini-icon mini-icon-text-file"></b></span>
                <span class="mode" title="File Mode">file</span>
                  <span>173 lines (129 sloc)</span>
                <span>6.041 kb</span>
              </div>
              <ul class="button-group actions">
                  <li>
                    <a class="grouped-button file-edit-link minibutton bigger lighter js-rewrite-sha" href="/ccotter/libdeterm/edit/dcc51ee8816a571258efd27bb8e9d04cd717603e/inc/fs_internal.h" data-method="post" rel="nofollow" data-hotkey="e">Edit</a>
                  </li>
                <li>
                  <a href="/ccotter/libdeterm/raw/master/inc/fs_internal.h" class="minibutton grouped-button bigger lighter" id="raw-url">Raw</a>
                </li>
                  <li>
                    <a href="/ccotter/libdeterm/blame/master/inc/fs_internal.h" class="minibutton grouped-button bigger lighter">Blame</a>
                  </li>
                <li>
                  <a href="/ccotter/libdeterm/commits/master/inc/fs_internal.h" class="minibutton grouped-button bigger lighter" rel="nofollow">History</a>
                </li>
              </ul>
            </div>
              <div class="data type-c">
      <table cellpadding="0" cellspacing="0" class="lines">
        <tr>
          <td>
            <pre class="line_numbers"><span id="L1" rel="#L1">1</span>
<span id="L2" rel="#L2">2</span>
<span id="L3" rel="#L3">3</span>
<span id="L4" rel="#L4">4</span>
<span id="L5" rel="#L5">5</span>
<span id="L6" rel="#L6">6</span>
<span id="L7" rel="#L7">7</span>
<span id="L8" rel="#L8">8</span>
<span id="L9" rel="#L9">9</span>
<span id="L10" rel="#L10">10</span>
<span id="L11" rel="#L11">11</span>
<span id="L12" rel="#L12">12</span>
<span id="L13" rel="#L13">13</span>
<span id="L14" rel="#L14">14</span>
<span id="L15" rel="#L15">15</span>
<span id="L16" rel="#L16">16</span>
<span id="L17" rel="#L17">17</span>
<span id="L18" rel="#L18">18</span>
<span id="L19" rel="#L19">19</span>
<span id="L20" rel="#L20">20</span>
<span id="L21" rel="#L21">21</span>
<span id="L22" rel="#L22">22</span>
<span id="L23" rel="#L23">23</span>
<span id="L24" rel="#L24">24</span>
<span id="L25" rel="#L25">25</span>
<span id="L26" rel="#L26">26</span>
<span id="L27" rel="#L27">27</span>
<span id="L28" rel="#L28">28</span>
<span id="L29" rel="#L29">29</span>
<span id="L30" rel="#L30">30</span>
<span id="L31" rel="#L31">31</span>
<span id="L32" rel="#L32">32</span>
<span id="L33" rel="#L33">33</span>
<span id="L34" rel="#L34">34</span>
<span id="L35" rel="#L35">35</span>
<span id="L36" rel="#L36">36</span>
<span id="L37" rel="#L37">37</span>
<span id="L38" rel="#L38">38</span>
<span id="L39" rel="#L39">39</span>
<span id="L40" rel="#L40">40</span>
<span id="L41" rel="#L41">41</span>
<span id="L42" rel="#L42">42</span>
<span id="L43" rel="#L43">43</span>
<span id="L44" rel="#L44">44</span>
<span id="L45" rel="#L45">45</span>
<span id="L46" rel="#L46">46</span>
<span id="L47" rel="#L47">47</span>
<span id="L48" rel="#L48">48</span>
<span id="L49" rel="#L49">49</span>
<span id="L50" rel="#L50">50</span>
<span id="L51" rel="#L51">51</span>
<span id="L52" rel="#L52">52</span>
<span id="L53" rel="#L53">53</span>
<span id="L54" rel="#L54">54</span>
<span id="L55" rel="#L55">55</span>
<span id="L56" rel="#L56">56</span>
<span id="L57" rel="#L57">57</span>
<span id="L58" rel="#L58">58</span>
<span id="L59" rel="#L59">59</span>
<span id="L60" rel="#L60">60</span>
<span id="L61" rel="#L61">61</span>
<span id="L62" rel="#L62">62</span>
<span id="L63" rel="#L63">63</span>
<span id="L64" rel="#L64">64</span>
<span id="L65" rel="#L65">65</span>
<span id="L66" rel="#L66">66</span>
<span id="L67" rel="#L67">67</span>
<span id="L68" rel="#L68">68</span>
<span id="L69" rel="#L69">69</span>
<span id="L70" rel="#L70">70</span>
<span id="L71" rel="#L71">71</span>
<span id="L72" rel="#L72">72</span>
<span id="L73" rel="#L73">73</span>
<span id="L74" rel="#L74">74</span>
<span id="L75" rel="#L75">75</span>
<span id="L76" rel="#L76">76</span>
<span id="L77" rel="#L77">77</span>
<span id="L78" rel="#L78">78</span>
<span id="L79" rel="#L79">79</span>
<span id="L80" rel="#L80">80</span>
<span id="L81" rel="#L81">81</span>
<span id="L82" rel="#L82">82</span>
<span id="L83" rel="#L83">83</span>
<span id="L84" rel="#L84">84</span>
<span id="L85" rel="#L85">85</span>
<span id="L86" rel="#L86">86</span>
<span id="L87" rel="#L87">87</span>
<span id="L88" rel="#L88">88</span>
<span id="L89" rel="#L89">89</span>
<span id="L90" rel="#L90">90</span>
<span id="L91" rel="#L91">91</span>
<span id="L92" rel="#L92">92</span>
<span id="L93" rel="#L93">93</span>
<span id="L94" rel="#L94">94</span>
<span id="L95" rel="#L95">95</span>
<span id="L96" rel="#L96">96</span>
<span id="L97" rel="#L97">97</span>
<span id="L98" rel="#L98">98</span>
<span id="L99" rel="#L99">99</span>
<span id="L100" rel="#L100">100</span>
<span id="L101" rel="#L101">101</span>
<span id="L102" rel="#L102">102</span>
<span id="L103" rel="#L103">103</span>
<span id="L104" rel="#L104">104</span>
<span id="L105" rel="#L105">105</span>
<span id="L106" rel="#L106">106</span>
<span id="L107" rel="#L107">107</span>
<span id="L108" rel="#L108">108</span>
<span id="L109" rel="#L109">109</span>
<span id="L110" rel="#L110">110</span>
<span id="L111" rel="#L111">111</span>
<span id="L112" rel="#L112">112</span>
<span id="L113" rel="#L113">113</span>
<span id="L114" rel="#L114">114</span>
<span id="L115" rel="#L115">115</span>
<span id="L116" rel="#L116">116</span>
<span id="L117" rel="#L117">117</span>
<span id="L118" rel="#L118">118</span>
<span id="L119" rel="#L119">119</span>
<span id="L120" rel="#L120">120</span>
<span id="L121" rel="#L121">121</span>
<span id="L122" rel="#L122">122</span>
<span id="L123" rel="#L123">123</span>
<span id="L124" rel="#L124">124</span>
<span id="L125" rel="#L125">125</span>
<span id="L126" rel="#L126">126</span>
<span id="L127" rel="#L127">127</span>
<span id="L128" rel="#L128">128</span>
<span id="L129" rel="#L129">129</span>
<span id="L130" rel="#L130">130</span>
<span id="L131" rel="#L131">131</span>
<span id="L132" rel="#L132">132</span>
<span id="L133" rel="#L133">133</span>
<span id="L134" rel="#L134">134</span>
<span id="L135" rel="#L135">135</span>
<span id="L136" rel="#L136">136</span>
<span id="L137" rel="#L137">137</span>
<span id="L138" rel="#L138">138</span>
<span id="L139" rel="#L139">139</span>
<span id="L140" rel="#L140">140</span>
<span id="L141" rel="#L141">141</span>
<span id="L142" rel="#L142">142</span>
<span id="L143" rel="#L143">143</span>
<span id="L144" rel="#L144">144</span>
<span id="L145" rel="#L145">145</span>
<span id="L146" rel="#L146">146</span>
<span id="L147" rel="#L147">147</span>
<span id="L148" rel="#L148">148</span>
<span id="L149" rel="#L149">149</span>
<span id="L150" rel="#L150">150</span>
<span id="L151" rel="#L151">151</span>
<span id="L152" rel="#L152">152</span>
<span id="L153" rel="#L153">153</span>
<span id="L154" rel="#L154">154</span>
<span id="L155" rel="#L155">155</span>
<span id="L156" rel="#L156">156</span>
<span id="L157" rel="#L157">157</span>
<span id="L158" rel="#L158">158</span>
<span id="L159" rel="#L159">159</span>
<span id="L160" rel="#L160">160</span>
<span id="L161" rel="#L161">161</span>
<span id="L162" rel="#L162">162</span>
<span id="L163" rel="#L163">163</span>
<span id="L164" rel="#L164">164</span>
<span id="L165" rel="#L165">165</span>
<span id="L166" rel="#L166">166</span>
<span id="L167" rel="#L167">167</span>
<span id="L168" rel="#L168">168</span>
<span id="L169" rel="#L169">169</span>
<span id="L170" rel="#L170">170</span>
<span id="L171" rel="#L171">171</span>
<span id="L172" rel="#L172">172</span>
</pre>
          </td>
          <td width="100%">
                <div class="highlight"><pre><div class='line' id='LC1'><br/></div><div class='line' id='LC2'><span class="cp">#ifndef _INC_FS_INTERNAL_H_</span></div><div class='line' id='LC3'><span class="cp">#define _INC_FS_INTERNAL_H_</span></div><div class='line' id='LC4'><br/></div><div class='line' id='LC5'><span class="cp">#include &lt;inc/types.h&gt;</span></div><div class='line' id='LC6'><span class="cp">#include &lt;inc/fs.h&gt;</span></div><div class='line' id='LC7'><span class="cp">#include &lt;inc/assert.h&gt;</span></div><div class='line' id='LC8'><br/></div><div class='line' id='LC9'><span class="cm">/* Deterministic file system (dfs for short) implemented in user space.</span></div><div class='line' id='LC10'><span class="cm">   This is essentially a replica of unix style inode file system. */</span></div><div class='line' id='LC11'><br/></div><div class='line' id='LC12'><span class="cp">#define PAGE_SIZE 0x1000</span></div><div class='line' id='LC13'><br/></div><div class='line' id='LC14'><span class="cm">/* Pick the size of a page since we can easily optomize using copy-on-write. */</span></div><div class='line' id='LC15'><span class="cp">#define FS_BLOCK_SIZE (PAGE_SIZE*1024)</span></div><div class='line' id='LC16'><br/></div><div class='line' id='LC17'><span class="cm">/* This determines the size in bytes of our file system (FS_BLOCK_SIZE * FS_NBLOCKS). */</span></div><div class='line' id='LC18'><span class="cp">#define FS_NBLOCKS (0x10000/1024 - 6)</span></div><div class='line' id='LC19'><br/></div><div class='line' id='LC20'><span class="cm">/* FS_BLOCK_SIZE PAGE_SIZE</span></div><div class='line' id='LC21'><span class="cm">   FS_NBLOCKS (0x10000 - 100)</span></div><div class='line' id='LC22'><span class="cm">   */</span></div><div class='line' id='LC23'><br/></div><div class='line' id='LC24'><span class="cm">/* Total number of bytes our file system can store. */</span></div><div class='line' id='LC25'><span class="cp">#define FS_SIZE (FS_NBLOCKS * FS_BLOCK_SIZE)</span></div><div class='line' id='LC26'><br/></div><div class='line' id='LC27'><span class="cm">/* A block can determine the in-use bit of FS_BLKBITSIZE blocks. */</span></div><div class='line' id='LC28'><span class="cp">#define FS_BLKBITSIZE (FS_BLOCK_SIZE * 8)</span></div><div class='line' id='LC29'><br/></div><div class='line' id='LC30'><span class="cm">/* How many blocks do we need for the bitmap? */</span></div><div class='line' id='LC31'><span class="cp">#define FS_BITMAP_BLOCKS ((FS_NBLOCKS + FS_BLKBITSIZE - 1) / FS_BLKBITSIZE)</span></div><div class='line' id='LC32'><br/></div><div class='line' id='LC33'><span class="cm">/* The inodes contain a fixed number of direct, indirect and doubly indirect blocks. */</span></div><div class='line' id='LC34'><span class="cp">#define FS_NDIRECT 10</span></div><div class='line' id='LC35'><span class="cm">/* Number of direct pointers in an indirect block. */</span></div><div class='line' id='LC36'><span class="cp">#define FS_NSINGLE_INDIRECT (FS_BLOCK_SIZE / sizeof(unsigned long))</span></div><div class='line' id='LC37'><span class="cm">/* Number of indirect pointers in a doubly indirect block. */</span></div><div class='line' id='LC38'><span class="cp">#define FS_NDOUBLE_INDIRECT (FS_BLOCK_SIZE / sizeof(unsigned long))</span></div><div class='line' id='LC39'><br/></div><div class='line' id='LC40'><span class="cm">/* We will pad the inode by the required amount to enforce this. */</span></div><div class='line' id='LC41'><span class="cp">#define FS_INODE_SIZE 128</span></div><div class='line' id='LC42'><br/></div><div class='line' id='LC43'><span class="k">struct</span> <span class="n">inode</span></div><div class='line' id='LC44'><span class="p">{</span></div><div class='line' id='LC45'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="cm">/* Size in bytes of this file. */</span></div><div class='line' id='LC46'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="kt">off_t</span> <span class="n">f_size</span><span class="p">;</span></div><div class='line' id='LC47'><br/></div><div class='line' id='LC48'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="cm">/* Number of times this inode is referenced by a directory file. */</span></div><div class='line' id='LC49'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="kt">uint32_t</span> <span class="n">f_refcount</span><span class="p">;</span></div><div class='line' id='LC50'><br/></div><div class='line' id='LC51'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="cm">/* Type of file (either directory or &quot;normal&quot; file). */</span></div><div class='line' id='LC52'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="kt">uint32_t</span> <span class="n">f_type</span><span class="p">;</span></div><div class='line' id='LC53'><br/></div><div class='line' id='LC54'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="cm">/* Direct blocks. A direct block is allocated if and only if the value is != 0. */</span></div><div class='line' id='LC55'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="n">blockno_t</span> <span class="n">f_direct</span><span class="p">[</span><span class="n">FS_NDIRECT</span><span class="p">];</span></div><div class='line' id='LC56'><br/></div><div class='line' id='LC57'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="cm">/* Pointer to the indirect block. */</span></div><div class='line' id='LC58'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="n">blockno_t</span> <span class="n">f_indirect</span><span class="p">;</span></div><div class='line' id='LC59'><br/></div><div class='line' id='LC60'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="cm">/* Pointer to the doubly indirect block. */</span></div><div class='line' id='LC61'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="n">blockno_t</span> <span class="n">f_double_indirect</span><span class="p">;</span></div><div class='line' id='LC62'><br/></div><div class='line' id='LC63'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="cm">/* We use versioning to manage the distributed nature of the file system. </span></div><div class='line' id='LC64'><span class="cm">       Version 0 indicates the file is not in use. Version numbers start at 1. */</span></div><div class='line' id='LC65'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="kt">uint32_t</span> <span class="n">f_version</span><span class="p">;</span></div><div class='line' id='LC66'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="kt">uint32_t</span> <span class="n">f_rversion</span><span class="p">;</span></div><div class='line' id='LC67'><br/></div><div class='line' id='LC68'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="cm">/* Keep track of how this inode was changed. */</span></div><div class='line' id='LC69'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="kt">uint32_t</span> <span class="n">f_flags</span><span class="p">;</span></div><div class='line' id='LC70'><br/></div><div class='line' id='LC71'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="cm">/* For files that have append only changes, keep track of the original file offset</span></div><div class='line' id='LC72'><span class="cm">       to know where to begin copying on a file system merge. */</span></div><div class='line' id='LC73'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="kt">off_t</span> <span class="n">f_append_off</span><span class="p">;</span></div><div class='line' id='LC74'><br/></div><div class='line' id='LC75'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="cm">/* Pad out to FS_INODE_SIZE. */</span></div><div class='line' id='LC76'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="kt">uint8_t</span> <span class="n">f_pad</span><span class="p">[</span><span class="n">FS_INODE_SIZE</span> <span class="o">-</span> <span class="k">sizeof</span><span class="p">(</span><span class="kt">size_t</span><span class="p">)</span> <span class="o">-</span> <span class="k">sizeof</span><span class="p">(</span><span class="kt">uint32_t</span><span class="p">)</span> <span class="o">*</span> <span class="mi">5</span> <span class="o">-</span> <span class="k">sizeof</span><span class="p">(</span><span class="n">blockno_t</span><span class="p">)</span> <span class="o">*</span> <span class="p">(</span><span class="mi">2</span> <span class="o">+</span> <span class="n">FS_NDIRECT</span><span class="p">)</span> <span class="o">-</span> <span class="k">sizeof</span><span class="p">(</span><span class="kt">off_t</span><span class="p">)];</span></div><div class='line' id='LC77'><br/></div><div class='line' id='LC78'><span class="p">}</span> <span class="n">__attribute__</span><span class="p">((</span><span class="n">packed</span><span class="p">));</span></div><div class='line' id='LC79'><br/></div><div class='line' id='LC80'><span class="n">static_assert</span><span class="p">(</span><span class="k">sizeof</span><span class="p">(</span><span class="k">struct</span> <span class="n">inode</span><span class="p">)</span> <span class="o">==</span> <span class="n">FS_INODE_SIZE</span><span class="p">);</span></div><div class='line' id='LC81'><br/></div><div class='line' id='LC82'><span class="cm">/* Flags that determine how an inode was changed. */</span></div><div class='line' id='LC83'><span class="cp">#define INODE_DELETED       0x0001</span></div><div class='line' id='LC84'><span class="cp">#define INODE_APPENDED      0x0002</span></div><div class='line' id='LC85'><br/></div><div class='line' id='LC86'><span class="cp">#define FS_MAGIC 0xdeadbeef</span></div><div class='line' id='LC87'><br/></div><div class='line' id='LC88'><span class="k">struct</span> <span class="n">super_block</span></div><div class='line' id='LC89'><span class="p">{</span></div><div class='line' id='LC90'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="kt">unsigned</span> <span class="kt">long</span> <span class="n">s_magic</span><span class="p">;</span> <span class="cm">/* Helps ensure validity of our file system. */</span></div><div class='line' id='LC91'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="kt">unsigned</span> <span class="kt">long</span> <span class="n">s_nblocks</span><span class="p">;</span> <span class="cm">/* How large our file system is. */</span></div><div class='line' id='LC92'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="k">struct</span> <span class="n">inode</span> <span class="o">*</span><span class="n">s_inode</span><span class="p">;</span> <span class="cm">/* Points to first inode. */</span></div><div class='line' id='LC93'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="n">ino_t</span> <span class="n">s_root</span><span class="p">;</span> <span class="cm">/* inode number of the root file inode (i.e. &quot;/&quot;). */</span></div><div class='line' id='LC94'><span class="p">};</span></div><div class='line' id='LC95'><br/></div><div class='line' id='LC96'><span class="cp">#define FS_VM_START 0x10000000</span></div><div class='line' id='LC97'><span class="cp">#define FS_VM_END (FS_VM_START + FS_SIZE)</span></div><div class='line' id='LC98'><span class="cp">#define FS_SCRATCH1 FS_VM_END</span></div><div class='line' id='LC99'><span class="cp">#define FS_SCRATCH1_SIZE ((6 - FS_BITMAP_BLOCKS - 4) * FS_BLOCK_SIZE)</span></div><div class='line' id='LC100'><span class="cp">#define FS_SCRATCH2 (FS_SCRATCH1 + FS_SCRATCH1_SIZE)</span></div><div class='line' id='LC101'><span class="cp">#define FS_SCRATCH2_SIZE (FS_BLOCK_SIZE * FS_BITMAP_BLOCKS)</span></div><div class='line' id='LC102'><span class="cp">#define FS_SCRATCH3 (FS_SCRATCH2 + FS_SCRATCH2_SIZE)</span></div><div class='line' id='LC103'><span class="cp">#define FS_SCRATCH3_SIZE FS_BLOCK_SIZE</span></div><div class='line' id='LC104'><span class="cp">#define FS_SCRATCH4 (FS_SCRATCH3 + FS_SCRATCH3_SIZE)</span></div><div class='line' id='LC105'><span class="cp">#define FS_SCRATCH4_SIZE FS_BLOCK_SIZE</span></div><div class='line' id='LC106'><span class="cp">#define FS_SCRATCH5 (FS_SCRATCH4 + FS_SCRATCH4_SIZE)</span></div><div class='line' id='LC107'><span class="cp">#define FS_SCRATCH5_SIZE FS_BLOCK_SIZE</span></div><div class='line' id='LC108'><span class="cp">#define FS_STATE (FS_SCRATCH5 + FS_SCRATCH4_SIZE)</span></div><div class='line' id='LC109'><span class="cp">#define FS_STATE_SIZE FS_BLOCK_SIZE</span></div><div class='line' id='LC110'><span class="cm">/* Must be a multiple of FS_BLOCK_SIZE / FS_INODE_SIZE */</span></div><div class='line' id='LC111'><span class="cp">#define FS_NINODES (0x8000)</span></div><div class='line' id='LC112'><br/></div><div class='line' id='LC113'><span class="n">static_assert</span><span class="p">(</span><span class="mi">0</span> <span class="o">==</span> <span class="n">FS_NINODES</span> <span class="o">%</span> <span class="p">(</span><span class="n">FS_BLOCK_SIZE</span> <span class="o">/</span> <span class="n">FS_INODE_SIZE</span><span class="p">));</span></div><div class='line' id='LC114'><span class="n">static_assert</span><span class="p">(</span><span class="mh">0x20000000</span> <span class="o">==</span> <span class="n">FS_STATE</span> <span class="o">+</span> <span class="n">FS_STATE_SIZE</span><span class="p">);</span></div><div class='line' id='LC115'><br/></div><div class='line' id='LC116'><span class="cp">#define NMAX_OPEN_FILES 10</span></div><div class='line' id='LC117'><span class="cp">#define FS_INTERNAL_FD NMAX_OPEN_FILES</span></div><div class='line' id='LC118'><br/></div><div class='line' id='LC119'><span class="cp">#define NDIRENTRIES_PER_BLOCK (FS_BLOCK_SIZE / sizeof(struct direntry))</span></div><div class='line' id='LC120'><br/></div><div class='line' id='LC121'><span class="cm">/* These structures define how our file system keeps internal state for a process&#39;s open files.</span></div><div class='line' id='LC122'><span class="cm">   We also have to keep track of the files we modify. This makes merging changes with the parent</span></div><div class='line' id='LC123'><span class="cm">   easier to manage. Our implementation, however, limits us to keeping track of a fixed number</span></div><div class='line' id='LC124'><span class="cm">   of modified files. This implies that a process can only modify a pre-determined number of files</span></div><div class='line' id='LC125'><span class="cm">   before it must sync with its parent. */</span></div><div class='line' id='LC126'><br/></div><div class='line' id='LC127'><span class="cm">/* This structure tracks specific files that a process opens. */</span></div><div class='line' id='LC128'><span class="k">struct</span> <span class="n">file_state_struct</span></div><div class='line' id='LC129'><span class="p">{</span></div><div class='line' id='LC130'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="cm">/* Uniquely determines the file we are working with. */</span></div><div class='line' id='LC131'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="k">struct</span> <span class="n">inode</span> <span class="o">*</span><span class="n">f_inode</span><span class="p">;</span></div><div class='line' id='LC132'><br/></div><div class='line' id='LC133'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="cm">/* Internal pointer to where we read from or write to. This can be set</span></div><div class='line' id='LC134'><span class="cm">       by a dfs_seek. */</span></div><div class='line' id='LC135'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="kt">off_t</span> <span class="n">f_offset</span><span class="p">;</span></div><div class='line' id='LC136'><br/></div><div class='line' id='LC137'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="cm">/* Read/Write/Append etc. */</span></div><div class='line' id='LC138'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="kt">int</span> <span class="n">f_mode</span><span class="p">;</span></div><div class='line' id='LC139'><span class="p">};</span></div><div class='line' id='LC140'><br/></div><div class='line' id='LC141'><span class="cm">/* Our &quot;file descriptors&quot; map directly into the s_openfiles array. */</span></div><div class='line' id='LC142'><span class="k">struct</span> <span class="n">dfs_state_struct</span></div><div class='line' id='LC143'><span class="p">{</span></div><div class='line' id='LC144'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="cm">/* Keeps track of currently opened files. Keep one extra spot open (+1)</span></div><div class='line' id='LC145'><span class="cm">       so that internal library code can still work with file descriptors. */</span></div><div class='line' id='LC146'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="k">struct</span> <span class="n">file_state_struct</span> <span class="n">s_openfiles</span><span class="p">[</span><span class="n">NMAX_OPEN_FILES</span> <span class="o">+</span> <span class="mi">1</span><span class="p">];</span></div><div class='line' id='LC147'><br/></div><div class='line' id='LC148'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="cm">/* This allows the parent to determine the runnable status of the child. */</span></div><div class='line' id='LC149'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="kt">int</span> <span class="n">proc_state</span><span class="p">;</span></div><div class='line' id='LC150'><br/></div><div class='line' id='LC151'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="kt">int</span> <span class="n">s_is_initialized</span><span class="p">;</span></div><div class='line' id='LC152'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="k">struct</span> <span class="n">super_block</span> <span class="o">*</span><span class="n">s_sblock</span><span class="p">;</span></div><div class='line' id='LC153'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="kt">uint8_t</span> <span class="o">*</span><span class="n">s_bmap</span><span class="p">;</span></div><div class='line' id='LC154'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="kt">uint8_t</span> <span class="o">*</span><span class="n">s_child_bmap</span><span class="p">;</span></div><div class='line' id='LC155'><br/></div><div class='line' id='LC156'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="cm">/* When we merge child files into a parent, we sometimes need to copy the</span></div><div class='line' id='LC157'><span class="cm">       indirect and direct blocks from the child into parent scratch space. We</span></div><div class='line' id='LC158'><span class="cm">       cache the blocks in special locations. Here we track the block number for</span></div><div class='line' id='LC159'><span class="cm">       each slot. */</span></div><div class='line' id='LC160'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="n">blockno_t</span> <span class="n">s_block_slot</span><span class="p">;</span></div><div class='line' id='LC161'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="n">blockno_t</span> <span class="n">s_indirect_slot</span><span class="p">;</span></div><div class='line' id='LC162'>&nbsp;&nbsp;&nbsp;&nbsp;<span class="n">blockno_t</span> <span class="n">s_double_indirect_slot</span><span class="p">;</span></div><div class='line' id='LC163'><br/></div><div class='line' id='LC164'><span class="p">};</span></div><div class='line' id='LC165'><br/></div><div class='line' id='LC166'><span class="cm">/* Define some simply static low level functions. */</span></div><div class='line' id='LC167'><span class="cp">#define blockaddr(blockno) ((uint8_t*)(FS_VM_START + (blockno) * FS_BLOCK_SIZE))</span></div><div class='line' id='LC168'><span class="cp">#define inodeaddr(ino) (super-&gt;s_inode + (ino))</span></div><div class='line' id='LC169'><span class="cp">#define inode2ino(i) (((unsigned long)i - (unsigned long)super-&gt;s_inode) / sizeof(struct inode))</span></div><div class='line' id='LC170'><br/></div><div class='line' id='LC171'><span class="cp">#endif</span></div><div class='line' id='LC172'><br/></div></pre></div>
          </td>
        </tr>
      </table>
  </div>

          </div>
        </div>
      </div>
    </div>

  </div>

<div class="frame frame-loading large-loading-area" style="display:none;" data-tree-list-url="/ccotter/libdeterm/tree-list/dcc51ee8816a571258efd27bb8e9d04cd717603e" data-blob-url-prefix="/ccotter/libdeterm/blob/dcc51ee8816a571258efd27bb8e9d04cd717603e">
  <img src="https://a248.e.akamai.net/assets.github.com/images/spinners/octocat-spinner-128.gif?1347543529" height="64" width="64">
</div>

        </div>
      </div>
      <div class="context-overlay"></div>
    </div>

      <div id="footer-push"></div><!-- hack for sticky footer -->
    </div><!-- end of wrapper - hack for sticky footer -->

      <!-- footer -->
      <div id="footer" >
        
  <div class="upper_footer">
     <div class="container clearfix">

       <!--[if IE]><h4 id="blacktocat_ie">GitHub Links</h4><![endif]-->
       <![if !IE]><h4 id="blacktocat">GitHub Links</h4><![endif]>

       <ul class="footer_nav">
         <h4>GitHub</h4>
         <li><a href="https://github.com/about">About</a></li>
         <li><a href="https://github.com/blog">Blog</a></li>
         <li><a href="https://github.com/features">Features</a></li>
         <li><a href="https://github.com/contact">Contact &amp; Support</a></li>
         <li><a href="https://github.com/training">Training</a></li>
         <li><a href="http://enterprise.github.com/">GitHub Enterprise</a></li>
         <li><a href="http://status.github.com/">Site Status</a></li>
       </ul>

       <ul class="footer_nav">
         <h4>Clients</h4>
         <li><a href="http://mac.github.com/">GitHub for Mac</a></li>
         <li><a href="http://windows.github.com/">GitHub for Windows</a></li>
         <li><a href="http://eclipse.github.com/">GitHub for Eclipse</a></li>
         <li><a href="http://mobile.github.com/">GitHub Mobile Apps</a></li>
       </ul>

       <ul class="footer_nav">
         <h4>Tools</h4>
         <li><a href="http://get.gaug.es/">Gauges: Web analytics</a></li>
         <li><a href="http://speakerdeck.com">Speaker Deck: Presentations</a></li>
         <li><a href="https://gist.github.com">Gist: Code snippets</a></li>

         <h4 class="second">Extras</h4>
         <li><a href="http://jobs.github.com/">Job Board</a></li>
         <li><a href="http://shop.github.com/">GitHub Shop</a></li>
         <li><a href="http://octodex.github.com/">The Octodex</a></li>
       </ul>

       <ul class="footer_nav">
         <h4>Documentation</h4>
         <li><a href="http://help.github.com/">GitHub Help</a></li>
         <li><a href="http://developer.github.com/">Developer API</a></li>
         <li><a href="http://github.github.com/github-flavored-markdown/">GitHub Flavored Markdown</a></li>
         <li><a href="http://pages.github.com/">GitHub Pages</a></li>
       </ul>

     </div><!-- /.site -->
  </div><!-- /.upper_footer -->

<div class="lower_footer">
  <div class="container clearfix">
    <!--[if IE]><div id="legal_ie"><![endif]-->
    <![if !IE]><div id="legal"><![endif]>
      <ul>
          <li><a href="https://github.com/site/terms">Terms of Service</a></li>
          <li><a href="https://github.com/site/privacy">Privacy</a></li>
          <li><a href="https://github.com/security">Security</a></li>
      </ul>

      <p>&copy; 2012 <span title="0.16513s from fe13.rs.github.com">GitHub</span> Inc. All rights reserved.</p>
    </div><!-- /#legal or /#legal_ie-->

  </div><!-- /.site -->
</div><!-- /.lower_footer -->

      </div><!-- /#footer -->

    

<div id="keyboard_shortcuts_pane" class="instapaper_ignore readability-extra" style="display:none">
  <h2>Keyboard Shortcuts <small><a href="#" class="js-see-all-keyboard-shortcuts">(see all)</a></small></h2>

  <div class="columns threecols">
    <div class="column first">
      <h3>Site wide shortcuts</h3>
      <dl class="keyboard-mappings">
        <dt>s</dt>
        <dd>Focus command bar</dd>
      </dl>
      <dl class="keyboard-mappings">
        <dt>?</dt>
        <dd>Bring up this help dialog</dd>
      </dl>
    </div><!-- /.column.first -->

    <div class="column middle" style='display:none'>
      <h3>Commit list</h3>
      <dl class="keyboard-mappings">
        <dt>j</dt>
        <dd>Move selection down</dd>
      </dl>
      <dl class="keyboard-mappings">
        <dt>k</dt>
        <dd>Move selection up</dd>
      </dl>
      <dl class="keyboard-mappings">
        <dt>c <em>or</em> o <em>or</em> enter</dt>
        <dd>Open commit</dd>
      </dl>
      <dl class="keyboard-mappings">
        <dt>y</dt>
        <dd>Expand URL to its canonical form</dd>
      </dl>
    </div><!-- /.column.first -->

    <div class="column last js-hidden-pane" style='display:none'>
      <h3>Pull request list</h3>
      <dl class="keyboard-mappings">
        <dt>j</dt>
        <dd>Move selection down</dd>
      </dl>
      <dl class="keyboard-mappings">
        <dt>k</dt>
        <dd>Move selection up</dd>
      </dl>
      <dl class="keyboard-mappings">
        <dt>o <em>or</em> enter</dt>
        <dd>Open issue</dd>
      </dl>
      <dl class="keyboard-mappings">
        <dt><span class="platform-mac">⌘</span><span class="platform-other">ctrl</span> <em>+</em> enter</dt>
        <dd>Submit comment</dd>
      </dl>
      <dl class="keyboard-mappings">
        <dt><span class="platform-mac">⌘</span><span class="platform-other">ctrl</span> <em>+</em> shift p</dt>
        <dd>Preview comment</dd>
      </dl>
    </div><!-- /.columns.last -->

  </div><!-- /.columns.equacols -->

  <div class="js-hidden-pane" style='display:none'>
    <div class="rule"></div>

    <h3>Issues</h3>

    <div class="columns threecols">
      <div class="column first">
        <dl class="keyboard-mappings">
          <dt>j</dt>
          <dd>Move selection down</dd>
        </dl>
        <dl class="keyboard-mappings">
          <dt>k</dt>
          <dd>Move selection up</dd>
        </dl>
        <dl class="keyboard-mappings">
          <dt>x</dt>
          <dd>Toggle selection</dd>
        </dl>
        <dl class="keyboard-mappings">
          <dt>o <em>or</em> enter</dt>
          <dd>Open issue</dd>
        </dl>
        <dl class="keyboard-mappings">
          <dt><span class="platform-mac">⌘</span><span class="platform-other">ctrl</span> <em>+</em> enter</dt>
          <dd>Submit comment</dd>
        </dl>
        <dl class="keyboard-mappings">
          <dt><span class="platform-mac">⌘</span><span class="platform-other">ctrl</span> <em>+</em> shift p</dt>
          <dd>Preview comment</dd>
        </dl>
      </div><!-- /.column.first -->
      <div class="column last">
        <dl class="keyboard-mappings">
          <dt>c</dt>
          <dd>Create issue</dd>
        </dl>
        <dl class="keyboard-mappings">
          <dt>l</dt>
          <dd>Create label</dd>
        </dl>
        <dl class="keyboard-mappings">
          <dt>i</dt>
          <dd>Back to inbox</dd>
        </dl>
        <dl class="keyboard-mappings">
          <dt>u</dt>
          <dd>Back to issues</dd>
        </dl>
        <dl class="keyboard-mappings">
          <dt>/</dt>
          <dd>Focus issues search</dd>
        </dl>
      </div>
    </div>
  </div>

  <div class="js-hidden-pane" style='display:none'>
    <div class="rule"></div>

    <h3>Issues Dashboard</h3>

    <div class="columns threecols">
      <div class="column first">
        <dl class="keyboard-mappings">
          <dt>j</dt>
          <dd>Move selection down</dd>
        </dl>
        <dl class="keyboard-mappings">
          <dt>k</dt>
          <dd>Move selection up</dd>
        </dl>
        <dl class="keyboard-mappings">
          <dt>o <em>or</em> enter</dt>
          <dd>Open issue</dd>
        </dl>
      </div><!-- /.column.first -->
    </div>
  </div>

  <div class="js-hidden-pane" style='display:none'>
    <div class="rule"></div>

    <h3>Network Graph</h3>
    <div class="columns equacols">
      <div class="column first">
        <dl class="keyboard-mappings">
          <dt><span class="badmono">←</span> <em>or</em> h</dt>
          <dd>Scroll left</dd>
        </dl>
        <dl class="keyboard-mappings">
          <dt><span class="badmono">→</span> <em>or</em> l</dt>
          <dd>Scroll right</dd>
        </dl>
        <dl class="keyboard-mappings">
          <dt><span class="badmono">↑</span> <em>or</em> k</dt>
          <dd>Scroll up</dd>
        </dl>
        <dl class="keyboard-mappings">
          <dt><span class="badmono">↓</span> <em>or</em> j</dt>
          <dd>Scroll down</dd>
        </dl>
        <dl class="keyboard-mappings">
          <dt>t</dt>
          <dd>Toggle visibility of head labels</dd>
        </dl>
      </div><!-- /.column.first -->
      <div class="column last">
        <dl class="keyboard-mappings">
          <dt>shift <span class="badmono">←</span> <em>or</em> shift h</dt>
          <dd>Scroll all the way left</dd>
        </dl>
        <dl class="keyboard-mappings">
          <dt>shift <span class="badmono">→</span> <em>or</em> shift l</dt>
          <dd>Scroll all the way right</dd>
        </dl>
        <dl class="keyboard-mappings">
          <dt>shift <span class="badmono">↑</span> <em>or</em> shift k</dt>
          <dd>Scroll all the way up</dd>
        </dl>
        <dl class="keyboard-mappings">
          <dt>shift <span class="badmono">↓</span> <em>or</em> shift j</dt>
          <dd>Scroll all the way down</dd>
        </dl>
      </div><!-- /.column.last -->
    </div>
  </div>

  <div class="js-hidden-pane" >
    <div class="rule"></div>
    <div class="columns threecols">
      <div class="column first js-hidden-pane" >
        <h3>Source Code Browsing</h3>
        <dl class="keyboard-mappings">
          <dt>t</dt>
          <dd>Activates the file finder</dd>
        </dl>
        <dl class="keyboard-mappings">
          <dt>l</dt>
          <dd>Jump to line</dd>
        </dl>
        <dl class="keyboard-mappings">
          <dt>w</dt>
          <dd>Switch branch/tag</dd>
        </dl>
        <dl class="keyboard-mappings">
          <dt>y</dt>
          <dd>Expand URL to its canonical form</dd>
        </dl>
      </div>
    </div>
  </div>

  <div class="js-hidden-pane" style='display:none'>
    <div class="rule"></div>
    <div class="columns threecols">
      <div class="column first">
        <h3>Browsing Commits</h3>
        <dl class="keyboard-mappings">
          <dt><span class="platform-mac">⌘</span><span class="platform-other">ctrl</span> <em>+</em> enter</dt>
          <dd>Submit comment</dd>
        </dl>
        <dl class="keyboard-mappings">
          <dt>escape</dt>
          <dd>Close form</dd>
        </dl>
        <dl class="keyboard-mappings">
          <dt>p</dt>
          <dd>Parent commit</dd>
        </dl>
        <dl class="keyboard-mappings">
          <dt>o</dt>
          <dd>Other parent commit</dd>
        </dl>
      </div>
    </div>
  </div>

  <div class="js-hidden-pane" style='display:none'>
    <div class="rule"></div>
    <h3>Notifications</h3>

    <div class="columns threecols">
      <div class="column first">
        <dl class="keyboard-mappings">
          <dt>j</dt>
          <dd>Move selection down</dd>
        </dl>
        <dl class="keyboard-mappings">
          <dt>k</dt>
          <dd>Move selection up</dd>
        </dl>
        <dl class="keyboard-mappings">
          <dt>o <em>or</em> enter</dt>
          <dd>Open notification</dd>
        </dl>
      </div><!-- /.column.first -->

      <div class="column second">
        <dl class="keyboard-mappings">
          <dt>e <em>or</em> shift i <em>or</em> y</dt>
          <dd>Mark as read</dd>
        </dl>
        <dl class="keyboard-mappings">
          <dt>shift m</dt>
          <dd>Mute thread</dd>
        </dl>
      </div><!-- /.column.first -->
    </div>
  </div>

</div>

    <div id="markdown-help" class="instapaper_ignore readability-extra">
  <h2>Markdown Cheat Sheet</h2>

  <div class="cheatsheet-content">

  <div class="mod">
    <div class="col">
      <h3>Format Text</h3>
      <p>Headers</p>
      <pre>
# This is an &lt;h1&gt; tag
## This is an &lt;h2&gt; tag
###### This is an &lt;h6&gt; tag</pre>
     <p>Text styles</p>
     <pre>
*This text will be italic*
_This will also be italic_
**This text will be bold**
__This will also be bold__

*You **can** combine them*
</pre>
    </div>
    <div class="col">
      <h3>Lists</h3>
      <p>Unordered</p>
      <pre>
* Item 1
* Item 2
  * Item 2a
  * Item 2b</pre>
     <p>Ordered</p>
     <pre>
1. Item 1
2. Item 2
3. Item 3
   * Item 3a
   * Item 3b</pre>
    </div>
    <div class="col">
      <h3>Miscellaneous</h3>
      <p>Images</p>
      <pre>
![GitHub Logo](/images/logo.png)
Format: ![Alt Text](url)
</pre>
     <p>Links</p>
     <pre>
http://github.com - automatic!
[GitHub](http://github.com)</pre>
<p>Blockquotes</p>
     <pre>
As Kanye West said:

> We're living the future so
> the present is our past.
</pre>
    </div>
  </div>
  <div class="rule"></div>

  <h3>Code Examples in Markdown</h3>
  <div class="col">
      <p>Syntax highlighting with <a href="http://github.github.com/github-flavored-markdown/" title="GitHub Flavored Markdown" target="_blank">GFM</a></p>
      <pre>
```javascript
function fancyAlert(arg) {
  if(arg) {
    $.facebox({div:'#foo'})
  }
}
```</pre>
    </div>
    <div class="col">
      <p>Or, indent your code 4 spaces</p>
      <pre>
Here is a Python code example
without syntax highlighting:

    def foo:
      if not bar:
        return true</pre>
    </div>
    <div class="col">
      <p>Inline code for comments</p>
      <pre>
I think you should use an
`&lt;addr&gt;` element here instead.</pre>
    </div>
  </div>

  </div>
</div>


    <div id="ajax-error-message" class="flash flash-error">
      <span class="mini-icon mini-icon-exclamation"></span>
      Something went wrong with that request. Please try again.
      <a href="#" class="mini-icon mini-icon-remove-close ajax-error-dismiss"></a>
    </div>

    <div id="logo-popup">
      <h2>Looking for the GitHub logo?</h2>
      <ul>
        <li>
          <h4>GitHub Logo</h4>
          <a href="http://github-media-downloads.s3.amazonaws.com/GitHub_Logos.zip"><img alt="Github_logo" src="https://a248.e.akamai.net/assets.github.com/images/modules/about_page/github_logo.png?1334862345" /></a>
          <a href="http://github-media-downloads.s3.amazonaws.com/GitHub_Logos.zip" class="minibutton download">Download</a>
        </li>
        <li>
          <h4>The Octocat</h4>
          <a href="http://github-media-downloads.s3.amazonaws.com/Octocats.zip"><img alt="Octocat" src="https://a248.e.akamai.net/assets.github.com/images/modules/about_page/octocat.png?1334862345" /></a>
          <a href="http://github-media-downloads.s3.amazonaws.com/Octocats.zip" class="minibutton download">Download</a>
        </li>
      </ul>
    </div>

    
    
    <span id='server_response_time' data-time='0.16745' data-host='fe13'></span>
    
  </body>
</html>

