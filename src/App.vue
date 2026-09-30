<script setup lang="ts">
import { ref, computed, onMounted } from 'vue'
import { Line } from 'vue-chartjs'
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
  Filler
} from 'chart.js'
import { useWeight } from './composables/useWeight'

ChartJS.register(
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
  Filler
)

const { targetWeight, today, todayRecord, diffFromTarget, progress,
        sortedRecords, chartData, setTarget, addOrUpdateWeight, updateRecord, deleteRecord } = useWeight()

const weightInput = ref('')
const targetInput = ref('')
const showTargetModal = ref(false)
const showHistoryModal = ref(false)
const editingDate = ref('')
const editWeight = ref('')

const displayDiff = computed(() => {
  if (diffFromTarget.value === null) return null
  const sign = diffFromTarget.value > 0 ? '+' : ''
  return sign + diffFromTarget.value.toFixed(1)
})

const chartOptions = {
  responsive: true,
  maintainAspectRatio: false,
  plugins: {
    legend: { display: false },
    tooltip: {
      callbacks: {
        label: (ctx: any) => `${ctx.parsed.y} kg`
      }
    }
  },
  scales: {
    y: {
      beginAtZero: false,
      suggestedMin: Math.min(...chartData.value.values) - 2,
      suggestedMax: Math.max(...chartData.value.values) + 2
    }
  }
}

const chartDataset = computed(() => ({
  labels: chartData.value.labels,
  datasets: [{
    data: chartData.value.values,
    borderColor: '#3b82f6',
    backgroundColor: 'rgba(59, 130, 246, 0.1)',
    fill: true,
    tension: 0.3,
    pointRadius: 4,
    pointBackgroundColor: '#3b82f6'
  }]
}))

function handleAddWeight() {
  const w = parseFloat(weightInput.value)
  if (isNaN(w) || w <= 0 || w > 500) {
    alert('请输入有效的体重值 (1-500 kg)')
    return
  }
  addOrUpdateWeight(w)
  weightInput.value = ''
}

function handleSetTarget() {
  const t = parseFloat(targetInput.value)
  if (isNaN(t) || t <= 0 || t > 500) {
    alert('请输入有效的目标体重 (1-500 kg)')
    return
  }
  setTarget(t)
  showTargetModal.value = false
  targetInput.value = ''
}

function openEditRecord(date: string, weight: number) {
  editingDate.value = date
  editWeight.value = weight.toString()
  showHistoryModal.value = false
  setTimeout(() => {
    const modal = document.getElementById('editModal')
    if (modal) (modal as any).showModal?.()
  }, 50)
}

function handleUpdateRecord() {
  const w = parseFloat(editWeight.value)
  if (isNaN(w) || w <= 0 || w > 500) {
    alert('请输入有效的体重值')
    return
  }
  updateRecord(editingDate.value, w)
  closeEditModal()
}

function handleDeleteRecord() {
  if (confirm(`确定删除 ${editingDate.value} 的记录吗？`)) {
    deleteRecord(editingDate.value)
    closeEditModal()
  }
}

function closeEditModal() {
  const modal = document.getElementById('editModal') as HTMLDialogElement | null
  if (modal) modal.close()
  editingDate.value = ''
  editWeight.value = ''
}

onMounted(() => {
  if (todayRecord.value) {
    weightInput.value = todayRecord.value.weight.toString()
  }
})
</script>

<template>
  <div class="app">
    <header class="header">
      <h1>⚖️ 体重监控</h1>
    </header>

    <main class="main">
      <!-- 目标差距卡片 -->
      <div class="card diff-card" v-if="diffFromTarget !== null">
        <div class="diff-label">距离目标还差</div>
        <div class="diff-value" :class="{ reached: diffFromTarget <= 0 }">
          {{ displayDiff }} kg
        </div>
        <div class="progress-bar">
          <div class="progress-fill" :style="{ width: progress + '%' }"></div>
        </div>
        <div class="progress-text">{{ progress?.toFixed(0) }}% 完成</div>
      </div>
      <div class="card diff-card" v-else>
        <div class="diff-label">尚未设置目标</div>
        <button class="btn-primary" @click="showTargetModal = true">设置目标体重</button>
      </div>

      <!-- 今日体重 -->
      <div class="card">
        <div class="card-title">今日体重</div>
        <div class="today-date">{{ today }}</div>
        <div class="today-weight" v-if="todayRecord">
          {{ todayRecord.weight }} kg
        </div>
        <div class="today-weight empty" v-else>
          尚未记录
        </div>
        <div class="input-group">
          <input
            type="number"
            v-model="weightInput"
            placeholder="输入体重 (kg)"
            step="0.1"
            min="1"
            max="500"
            @keyup.enter="handleAddWeight"
          />
          <button class="btn-primary" @click="handleAddWeight">
            {{ todayRecord ? '更新' : '记录' }}
          </button>
        </div>
      </div>

      <!-- 体重曲线图 -->
      <div class="card" v-if="chartData.values.length > 0">
        <div class="card-title">体重曲线</div>
        <div class="chart-container">
          <Line :data="chartDataset" :options="chartOptions" />
        </div>
      </div>

      <!-- 历史记录入口 -->
      <button class="btn-secondary" @click="showHistoryModal = true" v-if="sortedRecords.length > 0">
        查看历史记录 ({{ sortedRecords.length }})
      </button>

      <!-- 设置入口 -->
      <button class="btn-text" @click="showTargetModal = true">
        {{ targetWeight ? `目标体重: ${targetWeight} kg (点击修改)` : '设置目标体重' }}
      </button>
    </main>

    <!-- 目标设置弹窗 -->
    <dialog :open="showTargetModal" @close="showTargetModal = false" id="targetModal">
      <div class="modal-content">
        <h2>设置目标体重</h2>
        <div class="input-group vertical">
          <label>目标体重 (kg)</label>
          <input
            type="number"
            v-model="targetInput"
            :placeholder="targetWeight?.toString() || '输入目标体重'"
            step="0.1"
            min="1"
            max="500"
            @keyup.enter="handleSetTarget"
          />
        </div>
        <div class="modal-actions">
          <button class="btn-secondary" @click="showTargetModal = false">取消</button>
          <button class="btn-primary" @click="handleSetTarget">保存</button>
        </div>
      </div>
    </dialog>

    <!-- 历史记录弹窗 -->
    <dialog :open="showHistoryModal" id="historyModal">
      <div class="modal-content">
        <h2>历史记录</h2>
        <div class="history-list">
          <div
            v-for="record in sortedRecords"
            :key="record.date"
            class="history-item"
            @click="openEditRecord(record.date, record.weight)"
          >
            <span class="history-date">{{ record.date }}</span>
            <span class="history-weight">{{ record.weight }} kg</span>
          </div>
        </div>
        <div class="modal-actions">
          <button class="btn-primary" @click="showHistoryModal = false">关闭</button>
        </div>
      </div>
    </dialog>

    <!-- 编辑记录弹窗 -->
    <dialog id="editModal">
      <div class="modal-content">
        <h2>编辑记录</h2>
        <div class="edit-date">{{ editingDate }}</div>
        <div class="input-group vertical">
          <label>体重 (kg)</label>
          <input
            type="number"
            v-model="editWeight"
            step="0.1"
            min="1"
            max="500"
            @keyup.enter="handleUpdateRecord"
          />
        </div>
        <div class="modal-actions">
          <button class="btn-danger" @click="handleDeleteRecord">删除</button>
          <button class="btn-secondary" @click="closeEditModal">取消</button>
          <button class="btn-primary" @click="handleUpdateRecord">保存</button>
        </div>
      </div>
    </dialog>
  </div>
</template>

<style>
* {
  box-sizing: border-box;
  margin: 0;
  padding: 0;
}

body {
  font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
  background: #f5f5f7;
  color: #333;
  min-height: 100vh;
}

.app {
  max-width: 480px;
  margin: 0 auto;
  padding: 20px 16px;
  padding-bottom: 40px;
}

.header {
  text-align: center;
  margin-bottom: 24px;
}

.header h1 {
  font-size: 24px;
  font-weight: 600;
  color: #1d1d1f;
}

.main {
  display: flex;
  flex-direction: column;
  gap: 16px;
}

.card {
  background: white;
  border-radius: 16px;
  padding: 20px;
  box-shadow: 0 2px 8px rgba(0,0,0,0.08);
}

.card-title {
  font-size: 14px;
  color: #86868b;
  margin-bottom: 8px;
}

.diff-card {
  text-align: center;
  background: linear-gradient(135deg, #3b82f6, #2563eb);
  color: white;
}

.diff-label {
  font-size: 14px;
  opacity: 0.9;
}

.diff-value {
  font-size: 36px;
  font-weight: 700;
  margin: 8px 0;
}

.diff-value.reached {
  color: #86efac;
}

.progress-bar {
  background: rgba(255,255,255,0.3);
  border-radius: 8px;
  height: 8px;
  overflow: hidden;
  margin: 12px 0;
}

.progress-fill {
  background: white;
  height: 100%;
  border-radius: 8px;
  transition: width 0.3s ease;
}

.progress-text {
  font-size: 12px;
  opacity: 0.9;
}

.today-date {
  font-size: 12px;
  color: #86868b;
}

.today-weight {
  font-size: 32px;
  font-weight: 600;
  color: #1d1d1f;
}

.today-weight.empty {
  color: #86868b;
  font-size: 20px;
}

.input-group {
  display: flex;
  gap: 8px;
  margin-top: 12px;
}

.input-group.vertical {
  flex-direction: column;
}

.input-group input {
  flex: 1;
  padding: 12px 16px;
  border: 1px solid #d2d2d7;
  border-radius: 10px;
  font-size: 16px;
  outline: none;
  transition: border-color 0.2s;
}

.input-group input:focus {
  border-color: #3b82f6;
}

.input-group.vertical label {
  font-size: 14px;
  color: #86868b;
  margin-bottom: 6px;
}

.btn-primary {
  padding: 12px 24px;
  background: #3b82f6;
  color: white;
  border: none;
  border-radius: 10px;
  font-size: 16px;
  font-weight: 500;
  cursor: pointer;
  transition: background 0.2s;
}

.btn-primary:hover {
  background: #2563eb;
}

.btn-secondary {
  padding: 12px 24px;
  background: #e5e5ea;
  color: #1d1d1f;
  border: none;
  border-radius: 10px;
  font-size: 16px;
  cursor: pointer;
  transition: background 0.2s;
}

.btn-secondary:hover {
  background: #d2d2d7;
}

.btn-danger {
  padding: 12px 24px;
  background: #ef4444;
  color: white;
  border: none;
  border-radius: 10px;
  font-size: 16px;
  cursor: pointer;
}

.btn-text {
  background: none;
  border: none;
  color: #3b82f6;
  font-size: 14px;
  cursor: pointer;
  text-align: center;
  padding: 8px;
}

.chart-container {
  height: 200px;
  margin-top: 12px;
}

.btn-secondary.history-btn {
  width: 100%;
}

/* Dialog */
dialog {
  width: 90%;
  max-width: 400px;
  border: none;
  border-radius: 16px;
  padding: 0;
  box-shadow: 0 8px 32px rgba(0,0,0,0.2);
}

dialog::backdrop {
  background: rgba(0,0,0,0.4);
}

.modal-content {
  padding: 24px;
}

.modal-content h2 {
  font-size: 20px;
  margin-bottom: 20px;
  text-align: center;
}

.modal-actions {
  display: flex;
  gap: 8px;
  justify-content: flex-end;
  margin-top: 20px;
}

.history-list {
  max-height: 300px;
  overflow-y: auto;
}

.history-item {
  display: flex;
  justify-content: space-between;
  padding: 12px 0;
  border-bottom: 1px solid #e5e5ea;
  cursor: pointer;
}

.history-item:hover {
  background: #f5f5f7;
  margin: 0 -12px;
  padding: 12px;
}

.history-date {
  color: #86868b;
}

.history-weight {
  font-weight: 600;
}

.edit-date {
  text-align: center;
  color: #86868b;
  margin-bottom: 16px;
  font-size: 14px;
}

/* Dark mode */
@media (prefers-color-scheme: dark) {
  body {
    background: #1d1d1f;
    color: #f5f5f7;
  }

  .card {
    background: #2c2c2e;
  }

  .card-title {
    color: #98989d;
  }

  .today-date, .progress-text {
    color: #98989d;
  }

  .btn-secondary {
    background: #3a3a3c;
    color: #f5f5f7;
  }

  .input-group input {
    background: #1d1d1f;
    border-color: #3a3a3c;
    color: #f5f5f7;
  }

  .history-item {
    border-color: #3a3a3c;
  }

  .history-item:hover {
    background: #3a3a3c;
  }

  .modal-content h2, .history-date {
    color: #f5f5f7;
  }
}
</style>
