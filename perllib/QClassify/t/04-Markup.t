use Test::More qw(no_plan);
use strict;
BEGIN { use_ok('QClassify') };

my $n;
my $out;

my $qc2 = new QClassify("t/config4.xml");
$qc2->index2file(); # save index
$qc2->initMarkup(); # initialize markup: read config and load index file

my $str = "<div class=t75><div align=\"right\">Публикуется с любезного разрешения <font size=\"2\"><a href=\"http://www.tibetastromed.ru/\" target=\"_blank\"><strong>Института тибетской медицины и астрологии (г. Москва)</strong></a><br />Все комментарии на астрологические показатели сохранены без каких-либо изменений и полностью повторяют оригинальные тибетские источники (&laquo;Нагци нан сыт ме лонг&raquo; &laquo;Зеркало Бытия&raquo;,  &laquo;Вайдурья карпо&raquo; &laquo;Белый берилл&raquo;,  &laquo;Че сум так ци&raquo; &laquo;Три метода исследования&raquo; и др.), передавая атмосферу и образ мышления народов, населявших древний Тибет и царство Шанг-Шунг многие сотни лет назад. </font> </div>";

# default (config) marker will be used
($n, $out) = $qc2->markup($str);

#diag("Marked text: \"$out\"\n");

is($n, 3, "phrases should be marked");

my $scmp = "<div class=t75><div align=\"right\">Публикуется с любезного разрешения <font size=\"2\"><a href=\"http://www.tibetastromed.ru/\" target=\"_blank\"><strong>Института тибетской медицины и астрологии (г. Москва)</strong></a><br />Все комментарии на астрологические показатели сохранены без каких-либо изменений и полностью повторяют оригинальные тибетские источники (&laquo;Нагци нан сыт ме лонг&raquo; &laquo;Зеркало Бытия&raquo;,   <a href=\"#\">&laquo;Вайдурья карпо&raquo;</a> &laquo;Белый берилл&raquo;,   <i>&laquo;Че сум так ци&raquo;</i> &laquo;Три метода исследования&raquo; и др.), передавая атмосферу и образ мышления народов, населявших древний Тибет и <a class=\"gomail_search\" target=\"_blank\" href=\"http://go.mail.ru/search?q=царство Шанг-Шунг\">царство Шанг-Шунг</a>&nbsp;<img src=\"http://img.mail.ru/r/search_icon.gif\" width=\"13\" height=\"13\" alt=\"\" /> многие сотни лет назад. </font> </div>";


is($out, $scmp);

$str  = "Queen Богемская Рапсодия, 1974 год.";
$scmp = q#Queen <a href="http://go.mail.ru/search?q=%C1%EE%E3%E5%EC%F1%EA%E0%FF+%D0%E0%EF%F1%EE%E4%E8%FF">%D0%91%D0%BE%D0%B3%D0%B5%D0%BC%D1%81%D0%BA%D0%B0%D1%8F+%D0%A0%D0%B0%D0%BF%D1%81%D0%BE%D0%B4%D0%B8%D1%8F</a>, 1974 год.#;

($n, $out) = $qc2->markup($str);
is($n, 1, "phrases should be marked");
is($out, $scmp);

