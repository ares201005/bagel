!/
!/ Author : Toru Shiozaki
!/ Machine generated code
!/
      subroutine breitroot1(ta, rr, ww, n)
      implicit none
      integer i, j, n, offset, it, boxof
      double precision t, t2, d, e, f, g
      double precision ta(*), rr(*), ww(*)
      double precision ax(1)
      double precision aw(1)
      data (ax(i), i = 1, 1)/
     &   1.500000000000000e+00
     &/
      data (aw(i), i = 1, 1)/
     &   8.862269254527581e-01
     &/
      double precision x(384)
      double precision w(384)
      data x /
     &  1.057986693394197e+00,
     & -7.144282222753716e-02,
     & -2.054859453327102e-04,
     &  2.331146756779162e-04,
     &  1.331077830017425e-06,
     & -1.115171509111191e-06,
     & -8.999527162393619e-09,
     &  5.640469957441268e-09,
     &  5.868492293682324e-11,
     & -2.886120987688697e-11,
     & -3.681089987315752e-13,
     &  1.487463219474723e-13,
     &  7.851220690400758e-01,
     & -6.306370112940925e-02,
     &  2.077090515664856e-03,
     &  1.107583656237493e-04,
     & -1.277451547927649e-05,
     & -1.044487389335647e-08,
     &  6.009838083002671e-08,
     & -2.175115414441252e-09,
     & -2.080059059032380e-10,
     &  1.804354748413915e-11,
     &  3.447550363921192e-13,
     & -9.980335735361091e-14,
     &  5.700728455141461e-01,
     & -4.424389871474052e-02,
     &  2.353767967817790e-03,
     & -4.387325740927295e-05,
     & -5.008798807766955e-06,
     &  4.884288295089779e-07,
     & -1.173290746093463e-08,
     & -1.185932002907759e-09,
     &  1.202903061066997e-10,
     & -2.735219425546217e-12,
     & -3.083533159109398e-13,
     &  2.983920551688735e-14,
     &  4.263165133101116e-01,
     & -2.826877076462550e-02,
     &  1.603798639833974e-03,
     & -6.621801200235662e-05,
     &  9.705629527727786e-07,
     &  1.168434546975322e-07,
     & -1.170416650328710e-08,
     &  5.063515401445807e-10,
     & -9.010812787342137e-13,
     & -1.489830494944563e-12,
     &  1.095625215815169e-13,
     & -3.294304605340185e-15,
     &  3.343787695313898e-01,
     & -1.825217046123134e-02,
     &  9.421690541285707e-04,
     & -4.289019050586966e-05,
     &  1.506333938826640e-06,
     & -2.375199158095306e-08,
     & -1.642802891862454e-09,
     &  1.802845072073252e-10,
     & -9.557839910687163e-12,
     &  2.790196517090680e-13,
     &  1.998323378968711e-15,
     & -7.883517764092141e-16,
     &  2.737062959617326e-01,
     & -1.240744574659713e-02,
     &  5.529249190801726e-04,
     & -2.352162653738726e-05,
     &  9.051198513379506e-07,
     & -2.839050037196454e-08,
     &  5.109421933983436e-10,
     &  1.407039538947661e-11,
     & -1.949673343458228e-12,
     &  1.127750829710148e-13,
     & -4.374187315635119e-15,
     &  1.015963850541615e-16,
     &  2.314321486648044e-01,
     & -8.905431552066596e-03,
     &  3.411221934963245e-04,
     & -1.287092914971452e-05,
     &  4.676183879261686e-07,
     & -1.569514089798894e-08,
     &  4.499257284264834e-10,
     & -8.827025894078617e-12,
     & -4.528147109906610e-14,
     &  1.575661232617235e-14,
     & -9.771178325583664e-16,
     &  4.140295847801633e-17,
     &  2.004425977723634e-01,
     & -6.687523573427587e-03,
     &  2.228778169247477e-04,
     & -7.395888424548810e-06,
     &  2.423064077452070e-07,
     & -7.701382214546719e-09,
     &  2.300343695813390e-10,
     & -6.089986328575896e-12,
     &  1.239443061683193e-13,
     & -7.602553606340901e-16,
     & -9.582281811754662e-17,
     &  6.715820944592601e-18,
     &  1.767762141109114e-01,
     & -5.203610350329985e-03,
     &  1.531372602004397e-04,
     & -4.501650850226160e-06,
     &  1.318227678779673e-07,
     & -3.819713632881610e-09,
     &  1.080449216432170e-10,
     & -2.911511025958435e-12,
     &  7.155059166417748e-14,
     & -1.459122203961374e-15,
     &  1.714723604801397e-17,
     &  3.878715586059126e-19,
     &  1.581138140629937e-01,
     & -4.163747036495255e-03,
     &  1.096420218213107e-04,
     & -2.886384528312645e-06,
     &  7.590574699775137e-08,
     & -1.989587919128444e-09,
     &  5.170552283026493e-11,
     & -1.318402418151381e-12,
     &  3.237169261565035e-14,
     & -7.407527938951930e-16,
     &  1.480711182046233e-17,
     & -2.146133793937241e-19,
     &  1.430193785658226e-01,
     & -3.407151823652130e-03,
     &  8.116779623216395e-05,
     & -1.933526852665239e-06,
     &  4.604702000492541e-08,
     & -1.095581820289707e-09,
     &  2.599519200175242e-11,
     & -6.125793140220651e-13,
     &  1.422136136841771e-14,
     & -3.206123921352768e-16,
     &  6.847741816300389e-18,
     & -1.323580050468122e-19,
     &  1.305582405744536e-01,
     & -2.839564625134561e-03,
     &  6.175873588666825e-05,
     & -1.343196519956186e-06,
     &  2.921147589499571e-08,
     & -6.351259069163048e-10,
     &  1.379793425287233e-11,
     & -2.990809553309502e-13,
     &  6.447556601376684e-15,
     & -1.373798642932894e-16,
     &  2.861296678012272e-18,
     & -5.715160818258370e-20,
     &  1.200961151572054e-01,
     & -2.402883759028992e-03,
     &  4.807689503210470e-05,
     & -9.619199787150217e-07,
     &  1.924577355659989e-08,
     & -3.850393897402355e-10,
     &  7.701558903062076e-12,
     & -1.539414978130730e-13,
     &  3.071428657261740e-15,
     & -6.101756808188811e-17,
     &  1.201180640067729e-18,
     & -2.322474804736961e-20,
     &  1.111873974714942e-01,
     & -2.059732465291284e-03,
     &  3.815628079219078e-05,
     & -7.068398697088741e-07,
     &  1.309407210509106e-08,
     & -2.425616628010063e-10,
     &  4.493087025526283e-12,
     & -8.321161507172306e-14,
     &  1.540203208176981e-15,
     & -2.846673371464526e-17,
     &  5.243582978061400e-19,
     & -9.587378206994651e-21,
     &  1.035098338974698e-01,
     & -1.785183137531318e-03,
     &  3.078817421220887e-05,
     & -5.309884233881554e-07,
     &  9.157689161265787e-09,
     & -1.579375283561523e-10,
     &  2.723822044721043e-12,
     & -4.697317805182187e-14,
     &  8.099353198328178e-16,
     & -1.395889191299331e-17,
     &  2.402965421906549e-19,
     & -4.124487881562381e-21,
     &  9.682458365464176e-02,
     & -1.562093310508541e-03,
     &  2.520161113926371e-05,
     & -4.065833892823905e-07,
     &  6.559502515141727e-09,
     & -1.058258753430705e-10,
     &  1.707306223526556e-12,
     & -2.754389672061436e-14,
     &  4.443447182854592e-16,
     & -7.167301793964512e-18,
     &  1.155660357036138e-19,
     & -1.861209421046733e-21,
     &  9.095085938854892e-02,
     & -1.378359824587057e-03,
     &  2.088903632332902e-05,
     & -3.165732411070342e-07,
     &  4.797665769217359e-09,
     & -7.270858827993690e-11,
     &  1.101897296636707e-12,
     & -1.669918096052681e-14,
     &  2.530720658696433e-16,
     & -3.835100826358939e-18,
     &  5.811134365390768e-20,
     & -8.800694329041991e-22,
     &  8.574929257124383e-02,
     & -1.225239993899981e-03,
     &  1.750700207026308e-05,
     & -2.501510910788369e-07,
     &  3.574316597596288e-09,
     & -5.107208887892981e-11,
     &  7.297500866742994e-13,
     & -1.042712007914412e-14,
     &  1.489887058346579e-16,
     & -2.128815092662221e-18,
     &  3.041646382120743e-20,
     & -4.344618678335355e-22,
     &  8.111071056537980e-02,
     & -1.096290919106305e-03,
     &  1.481744853332377e-05,
     & -2.002723703977008e-07,
     &  2.706877787982895e-09,
     & -3.658611170781422e-11,
     &  4.944972139946927e-13,
     & -6.683614174622216e-15,
     &  9.033552960658669e-17,
     & -1.220969179453738e-18,
     &  1.650239748312328e-20,
     & -2.229968378756115e-22,
     &  7.694837640638635e-02,
     & -9.866798490756386e-04,
     &  1.265182152029981e-05,
     & -1.622295093272588e-07,
     &  2.080207474479467e-09,
     & -2.667371152017094e-11,
     &  3.420268843585154e-13,
     & -4.385680836693997e-15,
     &  5.623591072936953e-17,
     & -7.210911539448567e-19,
     &  9.246249596817101e-21,
     & -1.185404358058847e-22,
     &  7.319250547113997e-02,
     & -8.927243167395140e-04,
     &  1.088850150118277e-05,
     & -1.328063577050375e-07,
     &  1.619830666731022e-09,
     & -1.975697122970543e-11,
     &  2.409745164363792e-13,
     & -2.939150766386149e-15,
     &  3.584863264978267e-17,
     & -4.372433983921355e-19,
     &  5.333025633602838e-21,
     & -6.503672283796038e-23,
     &  6.978631577988531e-02,
     & -8.115785350685460e-04,
     &  9.438236009785576e-06,
     & -1.097617730474387e-07,
     &  1.276472299480924e-09,
     & -1.484470855413484e-11,
     &  1.726362351034134e-13,
     & -2.007669568920946e-15,
     &  2.334815204849949e-17,
     & -2.715268456353038e-19,
     &  3.157715275344596e-21,
     & -3.671759456771296e-23,
     &  6.668313367115805e-02,
     & -7.410152021126283e-04,
     &  8.234518978515185e-06,
     & -9.150595374319278e-08,
     &  1.016858372940869e-09,
     & -1.129982157788204e-11,
     &  1.255690773493143e-13,
     & -1.395384260947041e-15,
     &  1.550618411413408e-17,
     & -1.723122086361966e-19,
     &  1.914816411580889e-21,
     & -2.127573437600437e-23,
     &  6.384423980690615e-02,
     & -6.792709245887912e-04,
     &  7.227104440231730e-06,
     & -7.689279299218559e-08,
     &  8.181010338282292e-10,
     & -8.704187681385158e-12,
     &  9.260822326244377e-14,
     & -9.853053873845546e-16,
     &  1.048315875054867e-17,
     & -1.115355894803848e-19,
     &  1.186683113722182e-21,
     & -1.262428794520145e-23,
     &  6.123724356957945e-02,
     & -6.249349093931703e-04,
     &  6.377550951236128e-06,
     & -6.508382797035222e-08,
     &  6.641898584053275e-10,
     & -6.778153371822801e-12,
     &  6.917203349381316e-14,
     & -7.059105858388098e-16,
     &  7.203919416475140e-18,
     & -7.351703739951724e-20,
     &  7.502519680997237e-22,
     & -7.655632257320562e-24,
     &  5.883484054145521e-02,
     & -5.768676142156884e-04,
     &  5.656108545011271e-06,
     & -5.545737546117121e-08,
     &  5.437520281950621e-10,
     & -5.331414725409118e-12,
     &  5.227379669488312e-14,
     & -5.125374711271604e-16,
     &  5.025360236198257e-18,
     & -4.927297402477483e-20,
     &  4.831148080528969e-22,
     & -4.736419676927769e-24,
     &  5.661385170722979e-02,
     & -5.341404831787391e-04,
     &  5.039509716559036e-06,
     & -4.754677651870560e-08,
     &  4.485944237574224e-10,
     & -4.232399581222562e-12,
     &  3.993185217304065e-14,
     & -3.767491200602163e-16,
     &  3.554553363832264e-18,
     & -3.353650730252294e-20,
     &  3.164103047362650e-22,
     & -2.985002873487706e-24,
     &  5.455447255899810e-02,
     & -4.959907448952424e-04,
     &  4.509379478569656e-06,
     & -4.099774741974261e-08,
     &  3.727376020317001e-10,
     & -3.388803744408602e-12,
     &  3.080985324668412e-14,
     & -2.801127266954748e-16,
     &  2.546689821223788e-18,
     & -2.315363932946449e-20,
     &  2.105050273731409e-22,
     & -1.913682066835144e-24,
     &  5.263968047576973e-02,
     & -4.617871188746624e-04,
     &  4.051075941783504e-06,
     & -3.553848865704591e-08,
     &  3.117651197303763e-10,
     & -2.734992216986845e-12,
     &  2.399300612412218e-14,
     & -2.104811630895123e-16,
     &  1.846468082670482e-18,
     & -1.619833494965560e-20,
     &  1.421015917085596e-22,
     & -1.246505190398957e-24,
     &  5.085476277156078e-02,
     & -4.310035220859456e-04,
     &  3.652834580803000e-06,
     & -3.095844880833592e-08,
     &  2.623785806385094e-10,
     & -2.223707008192934e-12,
     &  1.884632825687528e-14,
     & -1.597261183497956e-16,
     &  1.353708400668723e-18,
     & -1.147292911749707e-20,
     &  9.723519614044817e-23,
     & -8.240270859322372e-25,
     &  4.918693768379647e-02,
     & -4.031987115847757e-04,
     &  3.305129546155459e-06,
     & -2.709304619038437e-08,
     &  2.220890714338660e-10,
     & -1.820524547286319e-12,
     &  1.492333506495387e-14,
     & -1.223306380531084e-16,
     &  1.002777525355174e-18,
     & -8.220040223452146e-21,
     &  6.738190601767012e-23,
     & -5.523107050623459e-25,
     &  4.762504762507144e-02,
     & -3.780003795005700e-04,
     &  3.000192000382501e-06,
     & -2.381254762508930e-08,
     &  1.890003787507110e-10,
     & -1.500097500289126e-12,
     &  1.190628571883334e-14,
     & -9.450028387566298e-17,
     &  7.500495001942498e-19,
     & -5.953148812565445e-21,
     &  4.725018900051778e-23,
     & -3.750015000040365e-25
     &/
      data w /
     &  1.545361991220441e+00,
     & -1.980219453337436e-01,
     &  2.612248383778893e-02,
     & -2.884849970973796e-03,
     &  2.670676766446337e-04,
     & -2.111301632244193e-05,
     &  1.451410808079440e-06,
     & -8.811300406217861e-08,
     &  4.784657763957309e-09,
     & -2.348708529668016e-10,
     &  1.051546525094701e-11,
     & -4.320417435690969e-13,
     &  1.023820359281469e+00,
     & -7.784518888153107e-02,
     &  7.624122364560783e-03,
     & -7.022396457247253e-04,
     &  5.777515301043659e-05,
     & -4.211209961005566e-06,
     &  2.730286060110980e-07,
     & -1.586297668351469e-08,
     &  8.326007203467479e-10,
     & -3.978265968632050e-11,
     &  1.742531851719414e-12,
     & -7.031246318392537e-14,
     &  7.970079097052309e-01,
     & -3.950969305784869e-02,
     &  2.805999935583594e-03,
     & -2.056406165829953e-04,
     &  1.441048334028490e-05,
     & -9.372763127162828e-07,
     &  5.594257726501581e-08,
     & -3.055973126482805e-09,
     &  1.530178901920561e-10,
     & -7.046806704995617e-12,
     &  2.997031479439444e-13,
     & -1.180746230988717e-14,
     &  6.723509527749635e-01,
     & -2.407591141126881e-02,
     &  1.278666772987362e-03,
     & -7.357837377974936e-05,
     &  4.261339325872135e-06,
     & -2.395863459410338e-07,
     &  1.280742031718389e-08,
     & -6.434556820643861e-10,
     &  3.021339691583471e-11,
     & -1.323410182233940e-12,
     &  5.409542892867429e-14,
     & -2.064337641047958e-15,
     &  5.921774787214273e-01,
     & -1.649894078348730e-02,
     &  6.877913937738493e-04,
     & -3.163574958092680e-05,
     &  1.504799656101738e-06,
     & -7.172254844666060e-08,
     &  3.353283031936781e-09,
     & -1.514471811286575e-10,
     &  6.537871344370493e-12,
     & -2.679765657446313e-13,
     &  1.039049344757623e-14,
     & -3.800835380518117e-16,
     &  5.352449052014906e-01,
     & -1.219218514606119e-02,
     &  4.163325660723200e-04,
     & -1.576962237759269e-05,
     &  6.243326116516219e-07,
     & -2.517955720870382e-08,
     &  1.017000834329550e-09,
     & -4.057418855429073e-11,
     &  1.580655434698953e-12,
     & -5.958121363380983e-14,
     &  2.158402842863696e-15,
     & -7.472574291998652e-17,
     &  4.921372047032724e-01,
     & -9.479878183151961e-03,
     &  2.738568678663404e-04,
     & -8.786876848549089e-06,
     &  2.956777650158546e-07,
     & -1.020287518570180e-08,
     &  3.563260205642908e-10,
     & -1.246570476841515e-11,
     &  4.327698839946231e-13,
     & -1.478079224134616e-14,
     &  4.928448858103256e-16,
     & -1.592741373606823e-17,
     &  4.580278917477482e-01,
     & -7.643351803961622e-03,
     &  1.913026746248399e-04,
     & -5.319549462679717e-06,
     &  1.552723756663200e-07,
     & -4.657842665839572e-09,
     &  1.420220856137482e-10,
     & -4.368115865709534e-12,
     &  1.346185680564329e-13,
     & -4.129861363466089e-15,
     &  1.252953887453511e-16,
     & -3.732672429585810e-18,
     &  4.301626319855134e-01,
     & -6.332087512750194e-03,
     &  1.398041568131519e-04,
     & -3.429549457989330e-06,
     &  8.833121026926291e-08,
     & -2.339558719248598e-09,
     &  6.307638480715125e-11,
     & -1.720268267518487e-12,
     &  4.723191540604572e-14,
     & -1.299666479568216e-15,
     &  3.567379420254427e-17,
     & -9.712342633616481e-19,
     &  4.068403203074891e-01,
     & -5.357338783229481e-03,
     &  1.058132946123642e-04,
     & -2.322103184678981e-06,
     &  5.350621529332044e-08,
     & -1.268058317257845e-09,
     &  3.060381583269722e-11,
     & -7.478857916501400e-13,
     &  1.843453137679334e-14,
     & -4.568561158765921e-16,
     &  1.134799885523821e-17,
     & -2.813955787273884e-19,
     &  3.869457918498814e-01,
     & -4.609438932599328e-03,
     &  8.236006753444615e-05,
     & -1.635074288179591e-06,
     &  3.408352301887205e-08,
     & -7.307693434479426e-10,
     &  1.595762271412849e-11,
     & -3.529478397242439e-13,
     &  7.879183135761312e-15,
     & -1.770786143609973e-16,
     &  3.997669791483280e-18,
     & -9.040996479033919e-20,
     &  3.697133073301491e-01,
     & -4.020761648868934e-03,
     &  6.558819925887582e-05,
     & -1.188765693690009e-06,
     &  2.262320897545976e-08,
     & -4.428379132057805e-10,
     &  8.828767269759333e-12,
     & -1.782979898833345e-13,
     &  3.635070252952328e-15,
     & -7.464382603070048e-17,
     &  1.541057727377835e-18,
     & -3.192404379706731e-20,
     &  3.545972105677661e-01,
     & -3.547569177209726e-03,
     &  5.323572820068557e-05,
     & -8.876247179971292e-07,
     &  1.553972717099624e-08,
     & -2.798278473492228e-10,
     &  5.132230631518549e-12,
     & -9.535029077804392e-14,
     &  1.788480718282900e-15,
     & -3.379290491986759e-17,
     &  6.421695848650021e-19,
     & -1.225306778286851e-20,
     &  3.411967022975044e-01,
     & -3.160448469635794e-03,
     &  4.391080913674853e-05,
     & -6.778733342867154e-07,
     &  1.098787502106924e-08,
     & -1.831946007605244e-10,
     &  3.110856865139695e-12,
     & -5.351189837294712e-14,
     &  9.293389920648787e-16,
     & -1.625907996565235e-17,
     &  2.861185375247947e-19,
     & -5.056922319011076e-21,
     &  3.292098455955971e-01,
     & -2.838965620737502e-03,
     &  3.672213684272063e-05,
     & -5.277771149836659e-07,
     &  7.964552625636050e-09,
     & -1.236249499084382e-10,
     &  1.954427017518791e-12,
     & -3.129942222956110e-14,
     &  5.060675142311431e-16,
     & -8.242972411395649e-18,
     &  1.350520259993636e-19,
     & -2.222586827188568e-21,
     &  3.184042560851505e-01,
     & -2.568528178016353e-03,
     &  3.107932965165038e-05,
     & -4.178439195246218e-07,
     &  5.898544866444772e-09,
     & -8.564651020552699e-11,
     &  1.266610574993063e-12,
     & -1.897493185005217e-14,
     &  2.869941607775614e-16,
     & -4.372909768412601e-18,
     &  6.702143230811388e-20,
     & -1.031862641003645e-21,
     &  3.085977624222850e-01,
     & -2.338465923669457e-03,
     &  2.657983403128008e-05,
     & -3.356826509261113e-07,
     &  4.451373082717253e-09,
     & -6.071459955125570e-11,
     &  8.434531516192718e-13,
     & -1.186950255239949e-14,
     &  1.686397321812041e-16,
     & -2.413749828901803e-18,
     &  3.475132607966003e-20,
     & -5.026093440963614e-22,
     &  2.996452602123106e-01,
     & -2.140814905389463e-03,
     &  2.294218073851218e-05,
     & -2.731781231413423e-07,
     &  3.415432424369102e-09,
     & -4.392174276824503e-11,
     &  5.752838066196600e-13,
     & -7.632880778750612e-15,
     &  1.022470235427727e-16,
     & -1.379805666996210e-18,
     &  1.872976029846456e-20,
     & -2.554097902798464e-22,
     &  2.914295431815748e-01,
     & -1.969523234659971e-03,
     &  1.996518270039426e-05,
     & -2.248750673757183e-07,
     &  2.659487552904296e-09,
     & -3.235107700584230e-11,
     &  4.008189679926263e-13,
     & -5.030504557084163e-15,
     &  6.374273706580461e-17,
     & -8.136824489734165e-19,
     &  1.044783501857993e-20,
     & -1.347713214613400e-22,
     &  2.838547630790503e-01,
     & -1.819918395618359e-03,
     &  1.750221247124857e-05,
     & -1.870208348975037e-07,
     &  2.098339398853319e-09,
     & -2.421562249457075e-11,
     &  2.846323770918188e-13,
     & -3.389040299769171e-15,
     &  4.074037809323376e-17,
     & -4.933765074685110e-19,
     &  6.010062883534241e-21,
     & -7.355068927764324e-23,
     &  2.768416714011880e-01,
     & -1.688341494046408e-03,
     &  1.544454080761653e-05,
     & -1.569805518002582e-07,
     &  1.675349663541768e-09,
     & -1.839074240675700e-11,
     &  2.056183339375972e-13,
     & -2.328778547089377e-15,
     &  2.662872932158896e-17,
     & -3.067452097404667e-19,
     &  3.554285278682001e-21,
     & -4.137521221618066e-23,
     &  2.703240961032394e-01,
     & -1.571890854351206e-03,
     &  1.371028172266994e-05,
     & -1.328699089684214e-07,
     &  1.352059193685121e-09,
     & -1.415138626920593e-11,
     &  1.508589411053573e-13,
     & -1.629096890588597e-15,
     &  1.776146056646394e-17,
     & -1.950812529177058e-19,
     &  2.155259404616725e-21,
     & -2.392227250896113e-23,
     &  2.642462914330190e-01,
     & -1.468238900706429e-03,
     &  1.223689804609203e-05,
     & -1.133188947685864e-07,
     &  1.101849225219773e-09,
     & -1.101986423698293e-11,
     &  1.122533006213132e-13,
     & -1.158312632422013e-15,
     &  1.206725172981286e-17,
     & -1.266473347969773e-19,
     &  1.336998418089759e-21,
     & -1.418042704999582e-23,
     &  2.585609161240256e-01,
     & -1.375499171216056e-03,
     &  1.097602153097627e-05,
     & -9.731639978166636e-08,
     &  9.059745683561973e-10,
     & -8.675214688454366e-12,
     &  8.460835830453805e-14,
     & -8.358918863822212e-16,
     &  8.337634678800394e-18,
     & -8.378007028366473e-20,
     &  8.468095320487228e-22,
     & -8.599207949724135e-24,
     &  2.532274711036311e-01,
     & -1.292128268021463e-03,
     &  9.889809525171255e-06,
     & -8.410596054878522e-08,
     &  7.510252613482619e-10,
     & -6.897895060091179e-12,
     &  6.452788681834095e-14,
     & -6.114797974048087e-16,
     &  5.850226732576242e-18,
     & -5.638562406406574e-20,
     &  5.466523848389668e-22,
     & -5.324580806101823e-24,
     &  2.482110781564149e-01,
     & -1.216852564609968e-03,
     &  8.948341339516080e-06,
     & -7.311453853152490e-08,
     &  6.272691018329120e-10,
     & -5.535263886881074e-12,
     &  4.974982931711862e-14,
     & -4.529483431856156e-16,
     &  4.163529768008436e-18,
     & -3.855491798583918e-20,
     &  3.591245193118691e-22,
     & -3.360816936633014e-24,
     &  2.434815149392183e-01,
     & -1.148612727030501e-03,
     &  8.127730415994788e-06,
     & -6.390305805720654e-08,
     &  5.275492013452836e-10,
     & -4.479593372741847e-12,
     &  3.874209080051296e-14,
     & -3.394152378594188e-16,
     &  3.002171780631940e-18,
     & -2.675129767759615e-20,
     &  2.397735847265430e-22,
     & -2.159211544398193e-24,
     &  2.390124450819236e-01,
     & -1.086521233152043e-03,
     &  7.408737175272946e-06,
     & -5.613153279355574e-08,
     &  4.465381974369900e-10,
     & -3.653798851221779e-12,
     &  3.045085554976376e-14,
     & -2.570740191228576e-16,
     &  2.191153484789428e-18,
     & -1.881449332549872e-20,
     &  1.625022666280811e-22,
     & -1.410153009024248e-24,
     &  2.347807984068971e-01,
     & -1.029829498326812e-03,
     &  6.775737205372990e-06,
     & -4.953413392933461e-08,
     &  3.802258285608564e-10,
     & -3.002015795420175e-12,
     &  2.414088466290127e-14,
     & -1.966514790012553e-16,
     &  1.617324552459839e-18,
     & -1.339992164216284e-20,
     &  1.116746263172624e-22,
     & -9.350804090186493e-25,
     &  2.307662678521608e-01,
     & -9.779021851156547e-04,
     &  6.215945044546663e-06,
     & -4.390113581220908e-08,
     &  3.255617586731961e-10,
     & -2.483278003735166e-12,
     &  1.929239533912196e-14,
     & -1.518276238358523e-16,
     &  1.206344549502993e-18,
     & -9.655995521597453e-21,
     &  7.774454324027257e-23,
     & -6.289078729732412e-25,
     &  2.269508979828176e-01,
     & -9.301969444882145e-04,
     &  5.718824102240073e-06,
     & -3.906568570243974e-08,
     &  2.802032836457072e-10,
     & -2.067213459112800e-12,
     &  1.553339147748772e-14,
     & -1.182363693244550e-16,
     &  9.086399219191553e-19,
     & -7.034571727399329e-21,
     &  5.478109173434597e-23,
     & -4.286174152758887e-25,
     &  2.233187460129850e-01,
     & -8.862483052146431e-04,
     &  5.275633701685654e-06,
     & -3.489400112802422e-08,
     &  2.423349082920835e-10,
     & -1.731073583404705e-12,
     &  1.259458737969899e-14,
     & -9.282312716269219e-17,
     &  6.906919268102137e-19,
     & -5.177471118114867e-21,
     &  3.903895319350381e-23,
     & -2.957511436211931e-25
     &/
      offset = -1
      do i=1, n
        t = ta(i)
        offset = offset + 1
        if (t < 0.0d0) then
          rr(offset+1:offset+1) = 0.5d0
          ww(offset+1:offset+1) = 0.0d0
        else if (t >= 64.0d0) then
          t = 1.0d0/dsqrt(t)
          rr(offset+1:offset+1) = ax(1:1)*t*t
          ww(offset+1:offset+1) = aw(1:1)*t
        else
          it = int(t*   0.500000000000000d0)
          t = (t-it*2.000000000000000-   1.000000000000000d0)
     &     *   1.000000000000000d0
          t2 = t * 2.0d0
          do j=1, 1
            boxof = it*12+12*(j-1)
            d = x(boxof+12)
            e = w(boxof+12)
            f = t2*d + x(boxof+11)
            g = t2*e + w(boxof+11)
            d = t2*f - d + x(boxof+10)
            e = t2*g - e + w(boxof+10)
            f = t2*d - f + x(boxof+9)
            g = t2*e - g + w(boxof+9)
            d = t2*f - d + x(boxof+8)
            e = t2*g - e + w(boxof+8)
            f = t2*d - f + x(boxof+7)
            g = t2*e - g + w(boxof+7)
            d = t2*f - d + x(boxof+6)
            e = t2*g - e + w(boxof+6)
            f = t2*d - f + x(boxof+5)
            g = t2*e - g + w(boxof+5)
            d = t2*f - d + x(boxof+4)
            e = t2*g - e + w(boxof+4)
            f = t2*d - f + x(boxof+3)
            g = t2*e - g + w(boxof+3)
            d = t2*f - d + x(boxof+2)
            e = t2*g - e + w(boxof+2)
            rr(offset+j) = t*d - f + x(boxof+1)*0.5d0
            ww(offset+j) = t*e - g + w(boxof+1)*0.5d0
          enddo
        endif
      enddo
      end subroutine