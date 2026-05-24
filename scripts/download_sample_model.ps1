param(
    [string]$ModelsDir = (Join-Path $PSScriptRoot "..\models"),
    [string]$ModelName = "yolov8n",
    [string]$ModelUrl = "https://github.com/ultralytics/assets/releases/download/v8.4.0/yolov8n.onnx",
    [string]$TaskType = "detection",
    [string]$Decoder = "yolo_v8",
    [string]$LabelsCategory = "coco80",
    [int]$InputSize = 640,
    [string]$ExpectedSha256 = ""
)

$ErrorActionPreference = "Stop"

function Test-DownloadedSha256 {
    param([string]$FilePath, [string]$ExpectedHash)
    if ([string]::IsNullOrWhiteSpace($ExpectedHash)) { return $true }
    $actual = (Get-FileHash -Path $FilePath -Algorithm SHA256).Hash.ToLower()
    if ($actual -ne $ExpectedHash.ToLower()) {
        Write-Warning "SHA256 mismatch: expected $ExpectedHash, got $actual"
        return $false
    }
    Write-Host "SHA256 verified."
    return $true
}

function Get-ChecksumFromManifest {
    param([string]$FileName, [string]$ModelsDirectory)
    $checksumsPath = Join-Path $ModelsDirectory "checksums.json"
    if (-not (Test-Path $checksumsPath)) { return $null }
    try {
        $checksums = Get-Content $checksumsPath -Raw | ConvertFrom-Json
        if ($checksums.files.PSObject.Properties.Name -contains $FileName) {
            $hash = $checksums.files.$FileName
            if (-not [string]::IsNullOrWhiteSpace($hash)) { return $hash }
        }
    } catch { }
    return $null
}

$ModelsDir = [System.IO.Path]::GetFullPath($ModelsDir)

if (-not (Test-Path $ModelsDir)) {
    New-Item -ItemType Directory -Path $ModelsDir -Force | Out-Null
    Write-Host "Created directory: $ModelsDir"
}

$onnxFileName = "$ModelName.onnx"
$onnxFilePath = Join-Path $ModelsDir $onnxFileName
$jsonFileName = "$ModelName.json"
$jsonFilePath = Join-Path $ModelsDir $jsonFileName

$cocoLabels = @(
    "person","bicycle","car","motorcycle","airplane","bus","train","truck","boat",
    "traffic light","fire hydrant","stop sign","parking meter","bench","bird","cat",
    "dog","horse","sheep","cow","elephant","bear","zebra","giraffe","backpack",
    "umbrella","handbag","tie","suitcase","frisbee","skis","snowboard","sports ball",
    "kite","baseball bat","baseball glove","skateboard","surfboard","tennis racket",
    "bottle","wine glass","cup","fork","knife","spoon","bowl","banana","apple",
    "sandwich","orange","broccoli","carrot","hot dog","pizza","donut","cake","chair",
    "couch","potted plant","bed","dining table","toilet","tv","laptop","mouse","remote",
    "keyboard","cell phone","microwave","oven","toaster","sink","refrigerator","book",
    "clock","vase","scissors","teddy bear","hair drier","toothbrush"
)

$imagenetLabels = @(
    "tench","goldfish","great white shark","tiger shark","hammerhead","electric ray",
    "stingray","cock","hen","ostrich","brambling","goldfinch","house finch","junco",
    "indigo bunting","robin","bulbul","jay","magpie","chickadee","water ouzel","kite",
    "bald eagle","vulture","great grey owl","European fire salamander","common newt",
    "eft","spotted salamander","axolotl","bullfrog","tree frog","tailed frog",
    "loggerhead","leatherback turtle","mud turtle","terrapin","box turtle","banded gecko",
    "common iguana","American chameleon","whiptail","agama","frilled lizard",
    "alligator lizard","Gila monster","green lizard","African chameleon","Komodo dragon",
    "African crocodile","American alligator","triceratops","thunder snake","ringneck snake",
    "hognose snake","green snake","king snake","garter snake","side-winder",
    "vine snake","night snake","boa constrictor","rock python","Indian cobra",
    "green mamba","sea snake","horned viper","diamondback","sidewinder","trilobite",
    "harvestman","scorpion","black and gold garden spider","barn spider","garden spider",
    "black widow","tarantula","wolf spider","tick","centipede","black grouse",
    "ptarmigan","ruffed grouse","prairie chicken","peacock","quail","partridge",
    "African grey","macaw","sulphur-crested cockatoo","lorikeet","coucal",
    "bee eater","hornbill","hummingbird","jacamar","toucan","drake","red-breasted merganser",
    "goose","black swan","tusker","echidna","platypus","wallaby","koala","wombat",
    "jellyfish","sea anemone","brain coral","flatworm","nematode","conch","snail","slug",
    "sea slug","chiton","chambered nautilus","Dungeness crab","rock crab","fiddler crab",
    "king crab","American lobster","spiny lobster","crayfish","hermit crab","isopod",
    "white stork","black stork","spoonbill","flamingo","little blue heron",
    "American egret","bittern","crane","limpkin","European gallinule",
    "American coot","bustard","ruddy turnstone","red-backed sandpiper","dowitcher",
    "oystercatcher","pelican","king penguin","albatross","grey whale","killer whale",
    "dugong","sea lion","Chihuahua","Japanese spaniel","Maltese dog","Pekinese",
    "Shih-Tzu","Blenheim spaniel","papillon","toy terrier","Rhodesian ridgeback",
    "Afghan hound","basset","beagle","bloodhound","bluetick","black-and-tan coonhound",
    "Walker hound","English foxhound","redbone","borzoi","Irish wolfhound",
    "Italian greyhound","whippet","Ibizan hound","Norwegian elkhound","otterhound",
    "Saluki","Scottish deerhound","Weimaraner","Staffordshire bullterrier",
    "American Staffordshire terrier","Bedlington terrier","Border terrier",
    "Kerry blue terrier","Irish terrier","Norfolk terrier","Norwich terrier",
    "Yorkshire terrier","wire-haired fox terrier","Lakeland terrier","Sealyham terrier",
    "Airedale","cairn","Australian terrier","Dandie Dinmont","Boston bull",
    "miniature schnauzer","giant schnauzer","standard schnauzer","Scotch terrier",
    "Tibetan terrier","silky terrier","soft-coated wheaten terrier","West Highland white terrier",
    "Lhasa","flat-coated retriever","curly-coated retriever","golden retriever",
    "Labrador retriever","Chesapeake Bay retriever","German short-haired pointer",
    "vizsla","English setter","Irish setter","Gordon setter","Brittany spaniel",
    "clumber","English springer","Welsh springer spaniel","cocker spaniel","Sussex spaniel",
    "Irish water spaniel","kuvasz","schipperke","groenendael","malinois","briard",
    "kelpie","komondor","Old English sheepdog","Shetland sheepdog","collie","Border collie",
    "Bouvier des Flandres","Rottweiler","German shepherd","Doberman","miniature pinscher",
    "Greater Swiss Mountain dog","Bernese mountain dog","Appenzeller","EntleBucher",
    "boxer","bull mastiff","Tibetan mastiff","French bulldog","Great Dane",
    "Saint Bernard","Eskimo dog","malamute","Siberian husky","dalmatian","affenpinscher",
    "basenji","pug","Leonberg","Newfoundland","Great Pyrenees","Samoyed","Pomeranian",
    "chow","keeshond","Brabancon griffon","Pembroke","Cardigan","toy poodle",
    "miniature poodle","standard poodle","Mexican hairless","timber wolf","white wolf",
    "red wolf","coyote","dingo","dhole","African hunting dog","hyena","red fox",
    "kit fox","Arctic fox","grey fox","tabby","tiger cat","Persian cat","Siamese cat",
    "Egyptian cat","cougar","lynx","leopard","snow leopard","jaguar","lion","tiger",
    "cheetah","brown bear","American black bear","ice bear","sloth bear","mongoose",
    "meerkat","tiger beetle","ladybug","ground beetle","long-horned beetle","leaf beetle",
    "dung beetle","rhinoceros beetle","weevil","fly","bee","ant","grasshopper",
    "cricket","walking stick","cockroach","mantis","cicada","leafhopper","lacewing",
    "dragonfly","damselfly","admiral","ringlet","monarch","cabbage butterfly",
    "sulphur butterfly","lycaenid","starfish","sea urchin","sea cucumber",
    "wood rabbit","hare","Angora","hamster","porcupine","fox squirrel","marmot",
    "beaver","guinea pig","sorrel","zebra","hog","wild boar","warthog","hippopotamus",
    "ox","water buffalo","bison","ram","bighorn","ibex","hartebeest","impala","gazelle",
    "Arabian camel","llama","weasel","mink","polecat","black-footed ferret","otter",
    "skunk","badger","armadillo","orangutan","gorilla","chimpanzee","gibbon","siamang",
    "guenon","patas","baboon","macaque","langur","colobus","proboscis monkey","marmoset",
    "capuchin","howler monkey","tit monkey","spider monkey","squirrel monkey",
    "Madagascar cat","indri","Indian elephant","African elephant","lesser panda",
    "giant panda","barracouta","eel","coho","rock beauty","clownfish","sturgeon",
    "gar","lionfish","puffer","abacus","abaya","academic gown","accordion","acoustic guitar",
    "aircraft carrier","airliner","airship","altar","ambulance","amphibian","analog clock",
    "apiary","apron","ashcan","assault rifle","backpack","bakery","balance beam",
    "balloon","ballpoint","Band-Aid","banjo","bannister","barbell","barber chair",
    "barbershop","barn","barometer","barrel","barrow","baseball","basketball",
    "bassinet","bassoon","bathing cap","bath towel","beacon","beaker","bearskin",
    "beer bottle","beer glass","bell cote","bib","bicycle-built-for-two","bikini",
    "binder","binoculars","birdhouse","boathouse","bobsled","bolo tie","bonnet","bookcase",
    "bookshop","bottlecap","bow","bow tie","brass","brassiere","breakwater","breastplate",
    "broom","bucket","buckle","bulletproof vest","bullet train","butcher shop",
    "cab","caldron","candle","cannon","canoe","can opener","cardigan","car mirror",
    "carousel","carpenter's kit","carton","car wheel","cash machine","cassette",
    "cassette player","castle","catamaran","CD player","cello","cellular telephone",
    "chain","chainlink fence","chain mail","chain saw","chest","chiffonier",
    "chime","china cabinet","Christmas stocking","church","cinema","cleaver","cliff dwelling",
    "cloak","clog","cocktail shaker","coffee mug","coffeepot","coil","combination lock",
    "computer keyboard","confectionery","container ship","convertible","corkscrew","cornet",
    "cowboy boot","cowboy hat","cradle","crane","crash helmet","crate","crib","Crock Pot",
    "croquet ball","crutch","cuirass","dam","desk","desktop computer","dial telephone",
    "diaper","digital clock","digital watch","dining table","dishrag","dishwasher",
    "disk brake","dock","dogsled","dome","doormat","drilling platform","drum","drumstick",
    "dumbbell","Dutch oven","electric fan","electric guitar","electric locomotive",
    "entertainment center","envelope","espresso maker","face powder","feather boa","file",
    "fireboat","fire engine","fire screen","flagpole","flute","folding chair",
    "football helmet","forklift","fountain","fountain pen","four-poster","freight car",
    "French horn","frying pan","fur coat","garbage truck","gasmask","gas pump","goblet",
    "go-kart","golf ball","golfcart","gondola","gong","gown","grand piano","greenhouse",
    "grille","grocery store","guillotine","hair slide","hair spray","half track","hammer",
    "hamper","hand blower","hand-held computer","handkerchief","hard disc","harmonica",
    "harp","harvester","hatchet","holster","home theater","honeycomb","hook","hoopskirt",
    "horizontal bar","horse cart","hourglass","iPod","iron","jack-o'-lantern","jean",
    "jeep","jersey","jigsaw puzzle","jinrikisha","joystick","kimono","knee pad","knot",
    "lab coat","ladle","lampshade","laptop","lawn mower","lens cap","letter opener",
    "library","lifeboat","lighter","limousine","liner","lipstick","Loafer","lotion",
    "loudspeaker","loupe","lumbermill","magnetic compass","mailbag","mailbox","maillot",
    "maillot","manhole cover","maraca","marimba","mask","matchstick","maypole","maze",
    "measuring cup","medicine chest","megalith","microphone","microwave","military uniform",
    "milk can","minibus","miniskirt","minivan","missile","mitten","mixing bowl",
    "mobile home","Model T","modem","monastery","monitor","moped","mortar","mortarboard",
    "mosque","mosquito net","motor scooter","mountain bike","mountain tent","mouse",
    "mousetrap","moving van","muzzle","nail","neck brace","necklace","nipple","notebook",
    "obelisk","oboe","ocarina","odometer","oil filter","organ","oscilloscope","overskirt",
    "oxcart","oxygen mask","packet","paddle","paddlewheel","padlock","paintbrush","pajama",
    "palace","panpipe","paper towel","parachute","parallel bars","park bench","parking meter",
    "passenger car","patio","pay-phone","pedestal","pencil box","pencil sharpener",
    "perfume","Petri dish","photocopier","pick","pickelhaube","picket fence","pickup",
    "pier","piggy bank","pill bottle","pillow","ping-pong ball","pinwheel","pirate",
    "pitcher","plane","planetarium","plastic bag","plate rack","plow","plunger","Polaroid camera",
    "pole","police van","poncho","pool table","pop bottle","pot","potter's wheel",
    "power drill","prayer rug","printer","prison","projectile","projector","puck",
    "punching bag","purse","quill","quilt","racer","racket","radiator","radio",
    "radio telescope","rain barrel","recreational vehicle","reel","reflex camera",
    "refrigerator","remote control","restaurant","revolver","rifle","rocking chair",
    "rotisserie","rubber eraser","rugby ball","rule","running shoe","safe","safety pin",
    "saltshaker","sandal","sarong","sax","scabbard","scale","school bus","schooner",
    "scoreboard","screen","screw","screwdriver","seat belt","sewing machine","shield",
    "shoe shop","shoji","shopping basket","shopping cart","shovel","shower cap",
    "shower curtain","ski","ski mask","sleeping bag","slide rule","sliding door",
    "slot","snorkel","snowmobile","snowplow","soap dispenser","soccer ball","sock",
    "solar dish","sombrero","soup bowl","space bar","space heater","space shuttle",
    "spatula","speedboat","spider web","spindle","sports car","spotlight","stage",
    "steam locomotive","steel arch bridge","steel drum","stethoscope","stole","stone wall",
    "stopwatch","stove","strainer","streetcar","stretcher","studio couch","stupa",
    "submarine","suit","sundial","sunglass","sunglasses","sunscreen","suspension bridge",
    "swab","sweatshirt","swimming trunks","swing","switch","syringe","table lamp",
    "tank","tape player","teapot","teddy","television","tennis ball","thatch","theater curtain",
    "thimble","thresher","throne","tile roof","toaster","tobacco shop","toilet seat",
    "torch","totem pole","tow truck","toyshop","tractor","trailer truck","tray",
    "trench coat","tricycle","trimaran","tripod","triumphal arch","trolleybus","trombone",
    "tub","turnstile","typewriter keyboard","umbrella","unicycle","upright","vacuum",
    "vase","vault","velvet","vending machine","vestment","viaduct","violin","volleyball",
    "waffle iron","wall clock","wallet","wardrobe","warplane","washbasin","washer",
    "water bottle","water jug","water tower","whiskey jug","whistle","wig","window screen",
    "window shade","Windsor tie","wine bottle","wing","wok","wooden spoon","wool",
    "worm fence","wreck","yawl","yurt","web site","comic book","crossword puzzle",
    "street sign","traffic light","book jacket","menu","plate","guacamole","consomme",
    "hot pot","trifle","ice cream","ice lolly","French loaf","bagel","pretzel","cheeseburger",
    "hotdog","mashed potato","head cabbage","broccoli","cauliflower","zucchini","acorn squash",
    "butternut squash","cucumber","artichoke","bell pepper","cardoon","mushroom",
    "Granny Smith","strawberry","orange","lemon","fig","pineapple","banana","jackfruit",
    "custard apple","pomegranate","hay","carbonara","chocolate sauce","dough","meat loaf",
    "pizza","potpie","burrito","red wine","espresso","cup","eggnog","alp","bubble",
    "cliff","coral reef","geyser","lakeside","promontory","sandbar","seashore","valley",
    "volcano","ballplayer","groom","scuba diver","rapeseed","daisy","yellow lady's slipper",
    "corn","acorn","hip","buckeye","coral fungus","agaric","gyromitra","stinkhorn",
    "earthstar","hen-of-the-woods","bolete","ear","toilet tissue"
)

$labels = @()
if ($LabelsCategory -eq "coco80") {
    $labels = $cocoLabels
} elseif ($LabelsCategory -eq "imagenet1000") {
    $labels = $imagenetLabels
}

$manifest = [ordered]@{
    name = $ModelName
    task_type = $TaskType
    backend = "onnxruntime"
    model = $onnxFileName
    input_width = $InputSize
    input_height = $InputSize
    confidence_threshold = 0.25
    nms_threshold = 0.45
}

if ($TaskType -eq "detection" -or $TaskType -eq "segmentation") {
    if (-not [string]::IsNullOrEmpty($Decoder)) {
        $manifest["decoder"] = $Decoder
    }
}

if ($labels.Count -gt 0) {
    $manifest["labels_inline"] = $labels
}

$jsonText = ConvertTo-Json -InputObject $manifest -Depth 3
Set-Content -Path $jsonFilePath -Value $jsonText -Encoding UTF8
Write-Host "Generated manifest: $jsonFilePath"

if (Test-Path $onnxFilePath) {
    Write-Host "Model file already exists: $onnxFilePath"
} else {
    if ([string]::IsNullOrWhiteSpace($ExpectedSha256)) {
        $ExpectedSha256 = Get-ChecksumFromManifest -FileName $onnxFileName -ModelsDirectory $ModelsDir
    }

    Write-Host ""
    Write-Host "Attempting to download $ModelName ONNX model..."
    Write-Host "  Source: $ModelUrl"
    Write-Host "  Target: $onnxFilePath"

    try {
        Invoke-WebRequest -Uri $ModelUrl -OutFile $onnxFilePath -UseBasicParsing
        $fileSize = (Get-Item $onnxFilePath).Length
        if ($fileSize -lt 1000000) {
            Remove-Item $onnxFilePath -Force -ErrorAction SilentlyContinue
            throw "Downloaded file too small ($fileSize bytes)"
        }
        if (-not (Test-DownloadedSha256 -FilePath $onnxFilePath -ExpectedHash $ExpectedSha256)) {
            Remove-Item $onnxFilePath -Force -ErrorAction SilentlyContinue
            throw "SHA256 verification failed"
        }
        Write-Host "Download complete, file size: $([math]::Round($fileSize / 1MB, 2)) MB"
    } catch {
        Write-Warning "Automatic download failed. Please download the model manually:"
        Write-Host ""
        Write-Host "  Option 1 - Direct download:"
        Write-Host "    Visit: $ModelUrl"
        Write-Host "    Save to: $onnxFilePath"
        Write-Host ""
        Write-Host "  Option 2 - Using pip (requires Python with ultralytics):"
        Write-Host "    pip install ultralytics"
        Write-Host "    yolo export model=$ModelName.pt format=onnx"
        Write-Host "    Copy the exported .onnx file to: $onnxFilePath"
        Write-Host ""
        Write-Host "  Option 3 - Using conda:"
        Write-Host "    conda install -c conda-forge ultralytics"
        Write-Host "    python -c `"from ultralytics import YOLO; YOLO('$ModelName.pt').export(format='onnx')`""
        Write-Host "    Copy the exported .onnx file to: $onnxFilePath"
    }
}

Write-Host ""
Write-Host "Usage:"
Write-Host "  1. Launch AI Toolkit"
Write-Host "  2. On the Models page, click 'Load Model Manifest'"
Write-Host "  3. Select: $jsonFilePath"
$taskLabel = switch ($TaskType) { "detection" { "Start Detection" } "classification" { "Start Classification" } "segmentation" { "Start Segmentation" } default { "Start Inference" } }
Write-Host "  4. Go to the Inference page, select an image, and click '$taskLabel'"
